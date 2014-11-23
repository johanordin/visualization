#version 400
layout(location = 0) in vec2 in_position;
layout(location = 1) in uint in_selection;

out float yPosition;

void main() {
    gl_Position = vec4(in_position, 0.0, 1.0);
    yPosition = in_position.y;
    bool isSelected = (in_selection == 1);
    if (isSelected)
    	gl_PointSize = 15.f; 
   	else
   		gl_PointSize = 1.f;
}
