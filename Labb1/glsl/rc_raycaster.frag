// variables for storing compositing results
vec4 result = vec4(0.0);

#define SAMPLING_BASE_INTERVAL_RCP 200.0

struct VOLUME_STRUCT {
    sampler3D volume_;              // the actual dataset normalized
    vec3 datasetDimensions_;        // the dataset's resolution, e.g. [ 256.0, 128.0, 128.0]
    vec3 datasetDimensionsRCP_;     // Reciprocal of the dataset dimension (= 1/datasetDimensions_)
};

struct TEXTURE_PARAMETERS {
    vec2 dimensions_;        // the texture's resolution, e.g. [256.0, 128.0]
    vec2 dimensionsRCP_;
};

struct LIGHT_SOURCE {
    vec3 position_;        // light position in world space
    vec3 ambientColor_;    // ambient color (r,g,b)
    vec3 diffuseColor_;    // diffuse color (r,g,b)
    vec3 specularColor_;   // specular color (r,g,b)
    vec3 attenuation_;     // attenuation (constant, linear, quadratic)
};

// uniforms needed for shading
uniform vec3 cameraPosition_;   // in world coordinates
uniform float shininess_;       // material shininess parameter
uniform LIGHT_SOURCE lightSource_;

// Settings for the raycaster
uniform float samplingStepSize_;
uniform float samplingRate_; 

// declare entry and exit parameters
uniform sampler2D entryPoints_;            // ray entry points
uniform TEXTURE_PARAMETERS entryParameters_;
uniform sampler2D exitPoints_;             // ray exit points
uniform TEXTURE_PARAMETERS exitParameters_;

// declare volume
uniform VOLUME_STRUCT volumeStruct_;    // volume data with parameters

// delcare transfer function
uniform sampler1D transferFunc_;

/////////////////////////////////////////////////////

vec3 calculateGradient(in vec3 samplePosition) {
    const vec3 h = volumeStruct_.datasetDimensionsRCP_;
    // Implement central differences
    
    float x1 = texture3D(volumeStruct_.volume_,samplePosition-h.x).a;
    float x2 = texture3D(volumeStruct_.volume_,samplePosition+h.x).a;
    float y1 = texture3D(volumeStruct_.volume_,samplePosition-h.y).a;
    float y2 = texture3D(volumeStruct_.volume_,samplePosition+h.y).a;
    float z1 = texture3D(volumeStruct_.volume_,samplePosition-h.z).a;
    float z2 = texture3D(volumeStruct_.volume_,samplePosition+h.z).a;
    //vec3 b = texture(volumeStruct_.volume_,vec3(samplePosition.x+samplingStepSize_,samplePosition.y,samplePosition.z)).rgb;
   

   vec3 gradient;
    gradient.x = (x2-x1)/(2*h.x);
    gradient.y = (y2-y1)/(2*h.y);
    gradient.z = (z2-z1)/(2*h.z);
    
    //return samplePosition;
    return normalize(gradient);
    
    //return vec3(0.0);
}

vec3 applyPhongShading(in vec3 pos, in vec3 gradient, in vec3 ka, in vec3 kd, in vec3 ks) {
    // Implement phong shading

    vec3 ambi;
    vec3 diff;
    vec3 spec;
    
    vec3 L = normalize(pos - lightSource_.position_);
    vec3 V = normalize(cameraPosition_ - pos);
    //vec3 R = normalize(2 * dot(gradient,L)*gradient-L);
    vec3 H = normalize(L+V);
    
    //vec3 R = 2*dot(gradient,L)*gradient - L;
    //vec3 V = pos - cameraPosition_;
    
    ambi = ka * lightSource_.ambientColor_;
    diff = kd * lightSource_.diffuseColor_ *max(dot(gradient,L), 0.f);
    spec = ks * lightSource_.specularColor_*pow(max(dot(gradient,H), 0.f), shininess_) ;
    //spec = ki*LIGHT_SOURCE*dot(R,V);
    
    return ambi + diff + spec;
    
    
    
    //return vec3(0.0);
}

void rayTraversal(in vec3 first, in vec3 last) {
    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection = last - first;
    tEnd = length(rayDirection);
    rayDirection = normalize(rayDirection);
    tIncr = 1.0/(samplingRate_ * length(rayDirection*volumeStruct_.datasetDimensions_));
    
    bool finished = false;
    while (!finished) {
        vec3 samplePos = first + t * rayDirection;
        float intensity = texture(volumeStruct_.volume_, samplePos).a;
        
        vec3 gradient = calculateGradient(samplePos);
	vec4 color = texture(transferFunc_, intensity);
	
	
        color.rgb = applyPhongShading(samplePos, gradient, color.rgb, color.rgb, vec3(1.0,1.0,1.0));


        // if opacity greater zero, apply compositing
        if (color.a > 0.0) {
            color.a = 1.0 - pow(1.0 - color.a, samplingStepSize_ * SAMPLING_BASE_INTERVAL_RCP);
            // Insert your front-to-back alpha compositing code here
            
	    result.rgb += (1.0 - result.a)*color.rgb*color.a;
            result.a += (1.0 - result.a)*color.a;

        }

        // early ray termination
        if (result.a > 1.0)
            finished = true;
        
        t += tIncr;
        finished = finished || (t > tEnd);
    }
}

void main() {
    vec3 frontPos = texture(entryPoints_, gl_FragCoord.xy * entryParameters_.dimensionsRCP_).rgb;
    vec3 backPos = texture(exitPoints_, gl_FragCoord.xy * exitParameters_.dimensionsRCP_).rgb;

    // determine whether the ray has to be casted
    if (frontPos == backPos)
        // background needs no raycasting
        discard;
    else
        // fragCoords are lying inside the bounding box
        rayTraversal(frontPos, backPos);

    FragData0 = result;
}
