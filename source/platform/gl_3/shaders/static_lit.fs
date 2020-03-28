#define LIGHT_COUNT 2

uniform sampler2D u_albedo;
uniform sampler2D u_specular;

uniform vec3 u_camPosition;

in vec3 v_normal;
in vec2 v_uv;
in vec3 v_lightPosition[2];
in vec3 v_fragPosition;

out vec4 fragColor;

void main()
{
    vec4 albedoColor = texture(u_albedo, v_uv.xy);
    vec3 specularColor = texture(u_specular, v_uv.xy).rgb;
    
    vec3 viewDir = normalize(v_fragPosition - u_camPosition);
    vec3 normal = normalize(v_normal);
    
    float diffuse = 0.0;
    float ambient = 0.2;
    float specular = 0.0;
    
    for (int i = 0; i < LIGHT_COUNT; ++i)
    {
        vec3 lightDir = normalize(v_lightPosition[i] - v_fragPosition);
        vec3 reflectionDir = reflect(lightDir, normal);
        
        float halfLambert = dot(normal, lightDir) * 0.5 + 0.5;
        diffuse += halfLambert * halfLambert;
        
        float cosAngle = max(dot(viewDir, reflectionDir), 0.0);
        specular += pow(cosAngle, 16.0);
    }
    
    vec3 col = (ambient + diffuse) * albedoColor.rgb + specular * specularColor.rgb;
    fragColor = vec4(col.r, col.g, col.b, albedoColor.a);
    
}

