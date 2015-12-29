attribute vec4 av4position;
attribute vec3 av3color;
uniform mat4 mvp;

varying vec3 vv3color;

void main() {
	// NOTE!! column major
	/*mat4 mvp = mat4(
		vec4(    1,    0,    0,    0),
		vec4(    0,    1,    0,    0),
		vec4(    0,    0,	-1,    0),
		vec4( -0.5,    0,    0,    1)
	);*/

	vv3color = av3color;
	gl_Position = mvp * av4position;
}

