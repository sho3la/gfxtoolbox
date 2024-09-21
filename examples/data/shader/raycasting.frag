#version 400

in vec3 EntryPoint;
in vec4 ExitPointCoord;

uniform sampler2D ExitPoints;
uniform sampler3D VolumeTex;

uniform float     StepSize;
uniform vec2      ScreenSize;
layout (location = 0) out vec4 FragColor;

		const float minIntensity = -1000; // Minimum intensity value
		const float maxIntensity = 3096; // Maximum intensity value

void main()
{
    vec3 exitPoint = texture(ExitPoints, gl_FragCoord.st/ScreenSize).xyz;

    if (EntryPoint == exitPoint)
    	discard;
		
    vec3 dir = exitPoint - EntryPoint;
    float len = length(dir); // the length from front to back is calculated and used to terminate the ray
    vec3 deltaDir = normalize(dir) * StepSize;
    float deltaDirLen = length(deltaDir);
    vec3 voxelCoord = EntryPoint;
	
    vec4 colorAcum = vec4(0.0); // The dest color
    float alphaAcum = 0.0; // The  dest alpha for blending


    float intensity;
    float lengthAcum = 0.0;
    vec4 colorSample; // The src color 

    // backgroundColor
    vec4 bgColor = vec4(1.0, 1.0, 1.0, 0.0);
 
    for(int i = 0; i < 1600; i++)
    {
    	intensity =  texture(VolumeTex, voxelCoord).x;
		
		float normalizedIntensity = (intensity - minIntensity) / (maxIntensity - minIntensity);
        normalizedIntensity = clamp(normalizedIntensity, 0.0, 1.0);
		
		float prev_alpha = normalizedIntensity * (1 - colorAcum.a);
		colorAcum.rgb += prev_alpha * vec3(normalizedIntensity, normalizedIntensity, normalizedIntensity);
		colorAcum.a += prev_alpha;
				
    	voxelCoord += deltaDir;
    	lengthAcum += deltaDirLen;
		
    	if (lengthAcum >= len )
    	{	
    	    colorAcum.rgb = colorAcum.rgb * colorAcum.a + (1 - colorAcum.a)*bgColor.rgb;		
    	    break;  // terminate if opacity > 1 or the ray is outside the volume	
    	}	
    	else if (colorAcum.a > 0.99)
    	{
    	    //colorAcum.a = 1.0;
    	    break;
    	}
    }
    FragColor = colorAcum;

   
}