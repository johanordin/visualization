vec3 hsv2rgb(vec3 inColor){
    float hh, p, q, t, ff;
    int i;
    vec3 outColor;

    hh = inColor.x;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = int(hh);
    ff = hh - i;
    p = inColor.z * (1.0 - inColor.y);
    q = inColor.z * (1.0 - (inColor.y * ff));
    t = inColor.z * (1.0 - (inColor.y * (1.0 - ff)));

    switch(i) {
    case 0:
        outColor.r = inColor.z;
        outColor.g = t;
        outColor.b = p;
        break;
    case 1:
        outColor.r = q;
        outColor.g = inColor.z;
        outColor.b = p;
        break;
    case 2:
        outColor.r = p;
        outColor.g = inColor.z;
        outColor.b = t;
        break;

    case 3:
        outColor.r = p;
        outColor.g = q;
        outColor.b = inColor.z;
        break;
    case 4:
        outColor.r = t;
        outColor.g = p;
        outColor.b = inColor.z;
        break;
    case 5:
    default:
        outColor.r = inColor.z;
        outColor.g = p;
        outColor.b = q;
        break;
    }
    return outColor;     
}


in float yPosition;

void main() {
	float normalizedPosition = (yPosition + 1.f) / 2.f;
	float hue = normalizedPosition * 360;
	vec3 hsvColor = vec3(hue, 0.85, 1.0);
	gl_FragData[0] = vec4(hsv2rgb(hsvColor), 1.0);
    // gl_FragData[0] = vec4(vec3(normalizedPosition),)
}
