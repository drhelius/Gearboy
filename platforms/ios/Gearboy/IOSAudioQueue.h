#ifndef IOS_AUDIO_QUEUE_H
#define IOS_AUDIO_QUEUE_H

#include <atomic>
#include <stdint.h>
#include <string.h>

class IOSAudioQueue
{
public:
    IOSAudioQueue() :
        m_startQueuedSamples(0),
        m_maxQueuedSamples(0),
        m_lastLeft(0.0f),
        m_lastRight(0.0f)
    {
        m_read.store(0, std::memory_order_relaxed);
        m_write.store(0, std::memory_order_relaxed);
        m_priming.store(true, std::memory_order_relaxed);
        m_fadeInFrames.store(0, std::memory_order_relaxed);
        Configure(2048, 3);
    }

    void Configure(uint32_t bufferSize, uint32_t bufferCount)
    {
        bufferSize &= ~1U;

        if (bufferSize < kChannelCount)
            bufferSize = kChannelCount;
        if (bufferSize > kCapacity)
            bufferSize = kCapacity;
        if (bufferCount < 1)
            bufferCount = 1;

        uint64_t maxQueuedSamples = (uint64_t)bufferSize * bufferCount;
        if (maxQueuedSamples > kCapacity)
            maxQueuedSamples = kCapacity;

        m_startQueuedSamples = bufferSize;
        m_maxQueuedSamples = (uint32_t)maxQueuedSamples;
        Reset();
    }

    void Reset()
    {
        uint32_t write = m_write.load(std::memory_order_acquire);
        m_read.store(write, std::memory_order_release);
        m_priming.store(true, std::memory_order_release);
        m_fadeInFrames.store(0, std::memory_order_release);
    }

    bool Write(const int16_t* samples, uint32_t count)
    {
        if (!samples)
            return false;

        count &= ~1U;
        if (count == 0)
            return false;

        uint32_t read = m_read.load(std::memory_order_acquire);
        uint32_t write = m_write.load(std::memory_order_relaxed);
        uint32_t queued = write - read;

        if ((queued >= m_maxQueuedSamples) || (count > (kCapacity - queued)))
            return false;

        for (uint32_t index = 0; index < count; ++index)
            m_samples[(write + index) & kMask] = samples[index];

        m_write.store(write + count, std::memory_order_release);
        return true;
    }

    bool Render(float* left, float* right, uint32_t frameCount)
    {
        return RenderInternal<false>(left, right, frameCount);
    }

    bool RenderInterleaved(float* output, uint32_t frameCount)
    {
        return RenderInternal<true>(output, 0, frameCount);
    }

    uint32_t GetQueuedSampleCount() const
    {
        uint32_t read = m_read.load(std::memory_order_acquire);
        uint32_t write = m_write.load(std::memory_order_acquire);
        return write - read;
    }

private:
    static const uint32_t kCapacity = 32768;
    static const uint32_t kMask = kCapacity - 1;
    static const uint32_t kChannelCount = 2;
    static const uint32_t kFadeFrames = 64;

    template <bool Interleaved>
    bool RenderInternal(float* outputA, float* outputB, uint32_t frameCount)
    {
        if (!outputA || (!Interleaved && !outputB) || (frameCount == 0))
            return false;

        uint32_t read = m_read.load(std::memory_order_relaxed);
        uint32_t write = m_write.load(std::memory_order_acquire);
        uint32_t availableSamples = write - read;
        uint32_t requestedSamples = frameCount * kChannelCount;

        if (m_priming.load(std::memory_order_acquire))
        {
            uint32_t requiredSamples = m_startQueuedSamples;
            if (requiredSamples < requestedSamples)
                requiredSamples = requestedSamples;
            if (requiredSamples > m_maxQueuedSamples)
                requiredSamples = m_maxQueuedSamples;

            if (availableSamples < requiredSamples)
            {
                ClearFrames<Interleaved>(outputA, outputB, 0, frameCount);
                m_lastLeft = 0.0f;
                m_lastRight = 0.0f;
                return false;
            }

            m_priming.store(false, std::memory_order_release);
        }

        uint32_t availableFrames = availableSamples / kChannelCount;
        uint32_t framesToRead = frameCount;
        if (framesToRead > availableFrames)
            framesToRead = availableFrames;

        uint32_t fadeInFrames = m_fadeInFrames.load(std::memory_order_acquire);
        float lastLeft = m_lastLeft;
        float lastRight = m_lastRight;

        for (uint32_t frame = 0; frame < framesToRead; ++frame)
        {
            float left = (float)m_samples[(read + (frame * kChannelCount)) & kMask] / 32768.0f;
            float right = (float)m_samples[(read + (frame * kChannelCount) + 1) & kMask] / 32768.0f;

            if (fadeInFrames < kFadeFrames)
            {
                float gain = (float)(fadeInFrames + 1) / (float)kFadeFrames;
                left *= gain;
                right *= gain;
                fadeInFrames++;
            }

            StoreFrame<Interleaved>(outputA, outputB, frame, left, right);
            lastLeft = left;
            lastRight = right;
        }

        uint32_t nextRead = read + (framesToRead * kChannelCount);
        m_read.compare_exchange_strong(read, nextRead, std::memory_order_release,
            std::memory_order_relaxed);
        m_fadeInFrames.store(fadeInFrames, std::memory_order_release);

        if (framesToRead < frameCount)
        {
            bool audible = FadeToSilence<Interleaved>(outputA, outputB, framesToRead,
                frameCount, lastLeft, lastRight);
            m_lastLeft = 0.0f;
            m_lastRight = 0.0f;
            m_priming.store(true, std::memory_order_release);
            m_fadeInFrames.store(0, std::memory_order_release);
            return (framesToRead > 0) || audible;
        }

        m_lastLeft = lastLeft;
        m_lastRight = lastRight;
        return framesToRead > 0;
    }

    template <bool Interleaved>
    bool FadeToSilence(float* outputA, float* outputB, uint32_t startFrame,
        uint32_t frameCount, float left, float right)
    {
        uint32_t remainingFrames = frameCount - startFrame;
        bool audible = ((left != 0.0f) || (right != 0.0f)) && (remainingFrames > 1);

        if (!audible)
        {
            ClearFrames<Interleaved>(outputA, outputB, startFrame, frameCount);
            return false;
        }

        uint32_t fadeFrames = remainingFrames;
        if (fadeFrames > kFadeFrames)
            fadeFrames = kFadeFrames;

        for (uint32_t frame = 0; frame < fadeFrames; ++frame)
        {
            float gain = 1.0f - ((float)(frame + 1) / (float)fadeFrames);
            StoreFrame<Interleaved>(outputA, outputB, startFrame + frame,
                left * gain, right * gain);
        }

        ClearFrames<Interleaved>(outputA, outputB, startFrame + fadeFrames, frameCount);
        return true;
    }

    template <bool Interleaved>
    void StoreFrame(float* outputA, float* outputB, uint32_t frame, float left, float right)
    {
        if (Interleaved)
        {
            outputA[frame * kChannelCount] = left;
            outputA[(frame * kChannelCount) + 1] = right;
        }
        else
        {
            outputA[frame] = left;
            outputB[frame] = right;
        }
    }

    template <bool Interleaved>
    void ClearFrames(float* outputA, float* outputB, uint32_t startFrame,
        uint32_t frameCount)
    {
        uint32_t clearFrames = frameCount - startFrame;

        if (Interleaved)
        {
            memset(outputA + (startFrame * kChannelCount), 0,
                clearFrames * kChannelCount * sizeof(float));
        }
        else
        {
            memset(outputA + startFrame, 0, clearFrames * sizeof(float));
            memset(outputB + startFrame, 0, clearFrames * sizeof(float));
        }
    }

private:
    int16_t m_samples[kCapacity];
    std::atomic<uint32_t> m_read;
    std::atomic<uint32_t> m_write;
    std::atomic<bool> m_priming;
    std::atomic<uint32_t> m_fadeInFrames;
    uint32_t m_startQueuedSamples;
    uint32_t m_maxQueuedSamples;
    float m_lastLeft;
    float m_lastRight;
};

#endif /* IOS_AUDIO_QUEUE_H */
