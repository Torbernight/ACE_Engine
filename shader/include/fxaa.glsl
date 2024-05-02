/**
--- copied and modified from https://github.com/mattdesl/glsl-fxaa/blob/master/fxaa.glsl ---

Basic FXAA implementation based on the code on geeks3d.com with the
modification that the texture2DLod stuff was removed since it's
unsupported by WebGL.

--

From:
https://github.com/mitsuhiko/webgl-meincraft

Copyright (c) 2011 by Armin Ronacher.

Some rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * The names of the contributors may not be used to endorse or
      promote products derived from this software without specific
      prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define FXAA_REDUCE_MIN   (1.0/ 128.0)
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#define FXAA_SPAN_MAX     8.0

vec4 fxaa(sampler2D tex, vec2 uv, vec2 resolution,
            vec2 v_rgbNW, vec2 v_rgbNE, 
            vec2 v_rgbSW, vec2 v_rgbSE, 
            vec2 v_rgbM) 
{
    vec4 color;
    vec2 inverseVP = vec2(1.0 / resolution.x, 1.0 / resolution.y);
    vec3 rgbNW = reinhard_extended_luminance(texture(tex, v_rgbNW).rgb, uExposure);
    vec3 rgbNE = reinhard_extended_luminance(texture(tex, v_rgbNE).rgb, uExposure);
    vec3 rgbSW = reinhard_extended_luminance(texture(tex, v_rgbSW).rgb, uExposure);
    vec3 rgbSE = reinhard_extended_luminance(texture(tex, v_rgbSE).rgb, uExposure);
    vec4 texColor = reinhard_extended_luminance(texture(tex, v_rgbM), uExposure);
    vec3 rgbM  = texColor.rgb;
    float lumaNW = luminance(rgbNW);
    float lumaNE = luminance(rgbNE);
    float lumaSW = luminance(rgbSW);
    float lumaSE = luminance(rgbSE);
    float lumaM  = luminance(rgbM);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
    
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
                          (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
              max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
              dir * rcpDirMin)) * inverseVP;
    
    vec3 rgbA = 0.5 * (
        reinhard_extended_luminance(texture(tex, uv + dir * (1.0 / 3.0 - 0.5)).rgb, uExposure) +
        reinhard_extended_luminance(texture(tex, uv + dir * (2.0 / 3.0 - 0.5)).rgb, uExposure));
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        reinhard_extended_luminance(texture(tex, uv + dir * -0.5).rgb, uExposure) +
        reinhard_extended_luminance(texture(tex, uv + dir * 0.5).rgb, uExposure));

    float lumaB = luminance(rgbB);
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        color = vec4(rgbA, texColor.a);
    else
        color = vec4(rgbB, texColor.a);
    return color;
}