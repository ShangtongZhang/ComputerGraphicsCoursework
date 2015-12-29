varying vec2 vv2texture;
varying vec4 vv4color;
uniform sampler2D tSource;
uniform float shaderParam;

void main() {
  
  vec2 vUv = vv2texture;
  vec2 delta = vec2(shaderParam, shaderParam);
	gl_FragColor =	texture2D(tSource, vUv);
	gl_FragColor.rgb = gl_FragColor.rgb * 0.25 +
		texture2D( tSource, vUv  + delta).rgb * 0.0625 +
		texture2D( tSource, vUv  - delta).rgb * 0.0625 +
		texture2D( tSource, vUv  + delta * vec2(1, -1)).rgb * 0.0625 +
		texture2D( tSource, vUv  + delta * vec2(- 1,1)).rgb * 0.0625 +

		texture2D( tSource, vUv  + delta * vec2(1, 0)).rgb * 0.125 +
		texture2D( tSource, vUv  - delta * vec2(0, 1)).rgb * 0.125 +
		texture2D( tSource, vUv  + delta * vec2(-1,0)).rgb * 0.125 +
		texture2D( tSource, vUv  + delta * vec2(0,-1)).rgb * 0.125;
  
  gl_FragColor = gl_FragColor * vv4color;
}