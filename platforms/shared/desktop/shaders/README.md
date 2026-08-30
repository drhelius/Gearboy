# Gearboy Shaders

Gearboy desktop shader presets use `.gshader` files. A preset is an INI-style file that describes one or more GLSL fragment shader passes. Presets placed in this `shaders` directory are discovered once when the desktop app starts and appear in the `Video > Shader` combo after `Pixel Perfect`.

GLSL files are normal fragment shaders. Gearboy supplies a fullscreen vertex shader and prepends the correct GLSL version, so shader files should not include a `#version` line.

## Minimal Preset

```ini
[Preset]
Name=Grid Bilinear
Passes=1

[Pass0]
Path=grid.glsl
ScaleType=Viewport
Filter=Linear

[Parameters]
GridIntensity=0.10

[Parameter.GridIntensity]
Label=Intensity
Min=0.0
Max=1.0
Step=0.01
```

## Preset Sections

`[Preset]`

- `Name`: display name shown in the menu. If omitted, the filename is used.
- `Passes`: number of passes, from 1 to 8.
- `SourcePalette`: optional source palette request. `Default` keeps the configured emulator palette, while `BlackWhite` temporarily uses Gearboy's internal black-and-white DMG palette while the preset is active.

`[PassN]`

- `Path`: GLSL fragment shader path. Relative paths are resolved from the `.gshader` file directory first, then from the `shaders` resource directory.
- `Filter`: `Nearest` by default. Use `Linear` or `Bilinear` for bilinear sampling on that pass input/output texture.
- `Feedback`: `true` when the pass needs the previous frame through `PassFeedback0`.
- `History`: `true` when the pass needs previous inputs to the same pass through `SourceHistory0` through `SourceHistory7`.
- `ScaleType`: sets both axes. Values are `Viewport`, `Source`, `Previous`, or `Absolute`.
- `ScaleTypeX`, `ScaleTypeY`: optional per-axis scale type overrides.
- `Scale`: output scale for both axes. Defaults to `1.0`.
- `ScaleX`, `ScaleY`: optional per-axis scale overrides.
- `AbsoluteWidth`, `AbsoluteHeight`: used with `ScaleType=Absolute`.
- `FloatFramebuffer`: `true` when the pass output needs signed or high-range intermediate values. Uses a floating-point render target.

The final pass is what Gearboy displays. Non-final passes render to intermediate textures.

`[Parameters]`

Each entry creates a float uniform with the same name:

```ini
[Parameters]
Amount=0.5
```

Optional metadata for sliders:

```ini
[Parameter.Amount]
Label=Amount
Min=0.0
Max=1.0
Step=0.01
```

A parameter with `Min=0.0`, `Max=1.0`, and `Step=1.0` is displayed as a checkbox. Other parameters with `Step=1.0` or higher are displayed as integer sliders.

A preset can expose up to 32 parameters.

## Shader Inputs

Available samplers:

- `uniform sampler2D Source`: current pass input.
- `uniform sampler2D Original`: original emulator frame.
- `uniform sampler2D PassFeedback0`: previous final output, for feedback presets.
- `uniform sampler2D SourceHistory0` through `SourceHistory7`: previous inputs to the same pass, newest first, for history presets.
- `uniform sampler2D PassOutput0` through `PassOutput3`: previous pass outputs, where `PassOutput0` is pass 0 output, `PassOutput1` is pass 1 output, and so on.

Available size uniforms are `vec4(width, height, 1.0 / width, 1.0 / height)`:

- `SourceSize`: current pass input size.
- `OriginalSize`: original emulator frame size.
- `OutputSize`: current pass output size.
- `FinalViewportSize`: final display viewport size.
- `PassFeedback0Size`: feedback texture size.
- `SourceHistorySize`: source history texture size.
- `PassOutputSize0` through `PassOutputSize3`: previous pass output texture sizes.

Other uniforms:

- `FrameCount`: increments once per rendered preset frame.
- `FrameDirection`: currently `1`.
- `SourceHistoryCount`: number of valid source history textures, from `0` to `8`.
- `OriginalAspect`: intended emulator image aspect ratio.
- `BackgroundColor`: configured normal desktop background color as `vec4(r, g, b, 1)`.

Any parameter declared in `[Parameters]` is also available as a `float` uniform.

## Minimal GLSL Shader

```glsl
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;

void main()
{
    FragColor = texture(Source, vTexCoord);
}
```

## Feedback Example

```ini
[Preset]
Name=Ghosting
Passes=1

[Pass0]
Path=ghosting.glsl
ScaleType=Viewport
Filter=Nearest
Feedback=true

[Parameters]
FeedbackAmount=0.5
```

A feedback shader can sample the previous final frame:

```glsl
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform sampler2D PassFeedback0;
uniform float FeedbackAmount;

void main()
{
    vec3 current = texture(Source, vTexCoord).rgb;
    vec3 previous = texture(PassFeedback0, vTexCoord).rgb;
    FragColor = vec4(mix(current, previous, FeedbackAmount), 1.0);
}
```
