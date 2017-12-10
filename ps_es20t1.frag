precision highp float;

uniform sampler2D sampler;
varying vec2 v_Texcoord;

void main (void)
{
    gl_FragColor = texture2D(sampler, v_Texcoord);
}
