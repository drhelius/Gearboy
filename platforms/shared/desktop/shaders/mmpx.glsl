// Copyright 2020 Morgan McGuire & Mara Gagiu,
// provided under the Open Source MIT license https://opensource.org/licenses/MIT
// Ported from mGBA's mmpx.shader/mmpx.fs to Gearboy GLSL preset uniforms.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;

float luma(vec4 color)
{
    return color.r + color.g + color.b + 1.0;
}

bool eq(vec4 left, vec4 right)
{
    return all(equal(left, right));
}

bool all_eq2(vec4 color, vec4 compare0, vec4 compare1)
{
    return all(bvec2(eq(color, compare0), eq(color, compare1)));
}

bool all_eq3(vec4 color, vec4 compare0, vec4 compare1, vec4 compare2)
{
    return all(bvec3(eq(color, compare0), eq(color, compare1), eq(color, compare2)));
}

bool all_eq4(vec4 color, vec4 compare0, vec4 compare1, vec4 compare2, vec4 compare3)
{
    return all(bvec4(eq(color, compare0), eq(color, compare1), eq(color, compare2), eq(color, compare3)));
}

bool any_eq2(vec4 color, vec4 compare0, vec4 compare1)
{
    return any(bvec2(eq(color, compare0), eq(color, compare1)));
}

bool any_eq3(vec4 color, vec4 compare0, vec4 compare1, vec4 compare2)
{
    return any(bvec3(eq(color, compare0), eq(color, compare1), eq(color, compare2)));
}

bool any_eq4(vec4 color, vec4 compare0, vec4 compare1, vec4 compare2, vec4 compare3)
{
    return any(bvec4(eq(color, compare0), eq(color, compare1), eq(color, compare2), eq(color, compare3)));
}

vec4 src(vec2 offset)
{
    vec2 coord = vTexCoord + offset * SourceSize.zw;
    return texture(Source, coord);
}

void main()
{
    vec4 A = src(vec2(-1.0, -1.0));
    vec4 B = src(vec2( 0.0, -1.0));
    vec4 C = src(vec2( 1.0, -1.0));

    vec4 D = src(vec2(-1.0,  0.0));
    vec4 E = src(vec2( 0.0,  0.0));
    vec4 F = src(vec2( 1.0,  0.0));

    vec4 G = src(vec2(-1.0,  1.0));
    vec4 H = src(vec2( 0.0,  1.0));
    vec4 I = src(vec2( 1.0,  1.0));

    vec4 J = E;
    vec4 K = E;
    vec4 L = E;
    vec4 M = E;

    bvec3 top = bvec3(eq(A, E), eq(B, E), eq(C, E));
    bvec3 mid = bvec3(eq(D, E), true,     eq(F, E));
    bvec3 bot = bvec3(eq(G, E), eq(H, E), eq(I, E));

    if (!all(top) || !all(mid) || !all(bot))
    {
        vec4 P = src(vec2( 0.0, -2.0));
        vec4 S = src(vec2( 0.0,  2.0));
        vec4 Q = src(vec2(-2.0,  0.0));
        vec4 R = src(vec2( 2.0,  0.0));
        float Bl = luma(B);
        float Dl = luma(D);
        float El = luma(E);
        float Fl = luma(F);
        float Hl = luma(H);

        if ((eq(D, B) && !eq(D, H) && !eq(D, F)) && (El >= Dl || eq(E, A)) && any_eq3(E, A, C, G) && ((El < Dl) || !eq(A, D) || !eq(E, P) || !eq(E, Q)))
            J = D;
        if ((eq(B, F) && !eq(B, D) && !eq(B, H)) && (El >= Bl || eq(E, C)) && any_eq3(E, A, C, I) && ((El < Bl) || !eq(C, B) || !eq(E, P) || !eq(E, R)))
            K = B;
        if ((eq(H, D) && !eq(H, F) && !eq(H, B)) && (El >= Hl || eq(E, G)) && any_eq3(E, A, G, I) && ((El < Hl) || !eq(G, H) || !eq(E, S) || !eq(E, Q)))
            L = H;
        if ((eq(F, H) && !eq(F, B) && !eq(F, D)) && (El >= Fl || eq(E, I)) && any_eq3(E, C, G, I) && ((El < Fl) || !eq(I, H) || !eq(E, R) || !eq(E, S)))
            M = F;

        if (!eq(E, F) && all_eq4(E, C, I, D, Q) && all_eq2(F, B, H) && !eq(F, src(vec2( 3.0,  0.0))))
            K = M = F;
        if (!eq(E, D) && all_eq4(E, A, G, F, R) && all_eq2(D, B, H) && !eq(D, src(vec2(-3.0,  0.0))))
            J = L = D;
        if (!eq(E, H) && all_eq4(E, G, I, B, P) && all_eq2(H, D, F) && !eq(H, src(vec2( 0.0,  3.0))))
            L = M = H;
        if (!eq(E, B) && all_eq4(E, A, C, H, S) && all_eq2(B, D, F) && !eq(B, src(vec2( 0.0, -3.0))))
            J = K = B;

        if (Bl < El && all_eq4(E, G, H, I, S) && !any_eq4(E, A, D, C, F))
            J = K = B;
        if (Hl < El && all_eq4(E, A, B, C, P) && !any_eq4(E, D, G, I, F))
            L = M = H;
        if (Fl < El && all_eq4(E, A, D, G, Q) && !any_eq4(E, B, C, I, H))
            K = M = F;
        if (Dl < El && all_eq4(E, C, F, I, R) && !any_eq4(E, B, A, G, H))
            J = L = D;

        if (!eq(H, B))
        {
            if (!eq(H, A) && !eq(H, E) && !eq(H, C))
            {
                if (all_eq3(H, G, F, R) && !any_eq2(H, D, src(vec2( 2.0, -1.0))))
                    L = M;
                if (all_eq3(H, I, D, Q) && !any_eq2(H, F, src(vec2(-2.0, -1.0))))
                    M = L;
            }

            if (!eq(B, I) && !eq(B, G) && !eq(B, E))
            {
                if (all_eq3(B, A, F, R) && !any_eq2(B, D, src(vec2( 2.0,  1.0))))
                    J = K;
                if (all_eq3(B, C, D, Q) && !any_eq2(B, F, src(vec2(-2.0,  1.0))))
                    K = J;
            }
        }

        if (!eq(F, D))
        {
            if (!eq(D, I) && !eq(D, E) && !eq(D, C))
            {
                if (all_eq3(D, A, H, S) && !any_eq2(D, B, src(vec2( 1.0, 2.0))))
                    J = L;
                if (all_eq3(D, G, B, P) && !any_eq2(D, H, src(vec2( 1.0, -2.0))))
                    L = J;
            }

            if (!eq(F, E) && !eq(F, A) && !eq(F, G))
            {
                if (all_eq3(F, C, H, S) && !any_eq2(F, B, src(vec2(-1.0,  2.0))))
                    K = M;
                if (all_eq3(F, I, B, P) && !any_eq2(F, H, src(vec2(-1.0, -2.0))))
                    M = K;
            }
        }
    }

    vec2 texel = fract(vTexCoord * SourceSize.xy);
    if (texel.x < 0.5)
        FragColor = texel.y < 0.5 ? J : L;
    else
        FragColor = texel.y < 0.5 ? K : M;
}