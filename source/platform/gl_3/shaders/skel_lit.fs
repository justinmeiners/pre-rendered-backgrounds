
#define LIGHT_COUNT 2

uniform sampler2D u_albedo;
uniform sampler2D u_normal;
uniform sampler2D u_specular;
uniform sampler2D u_gloss;
uniform samplerCube u_envMap;

uniform vec3 u_camPosition;

uniform vec3 u_lightPositions[2];

in vec2 v_uvs[1];
in vec3 v_fragPosition;
in mat3 v_tbnMatrix;


out vec4 fragColor;


void main()
{
    vec3 normal = v_tbnMatrix * normalize(texture(u_normal, v_uvs[0].xy).rgb * 2.0 - 1.0);
    vec3 viewDir = normalize(v_fragPosition - u_camPosition);

    vec3 albedoColor = texture(u_albedo, v_uvs[0].xy).rgb;
    vec3 specularColor = texture(u_specular, v_uvs[0].xy).rgb;
    
    vec3 envVec = reflect(viewDir, normal);
    vec3 envColor = texture(u_envMap, vec3(envVec.x, -envVec.y, envVec.z)).rgb;

    float gloss = texture(u_gloss, v_uvs[0].xy).r;
    
    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    float specFactor = 0.0;
    
    for (int i = 0; i < LIGHT_COUNT; ++i)
    {
        vec3 lightDir = normalize(u_lightPositions[i] - v_fragPosition);
        vec3 reflectionDir = reflect(lightDir, normal);
        
        float halfLambert = dot(normal, lightDir) * 0.5 + 0.5;

        diffuse += halfLambert * halfLambert * vec3(1.0, 1.0, 1.0);
        
        float cosAngle = max(dot(viewDir, reflectionDir), 0.0);
        specFactor += pow(cosAngle, (gloss + 0.01) * 72.0);
    }
    
    vec3 ambient = vec3(0.2, 0.2, 0.2);
    
    vec3 col = clamp(ambient + diffuse, 0.0, 1.0) * mix(albedoColor, envColor * albedoColor, gloss);
    col += specFactor * specularColor;
    fragColor = vec4(col.r, col.g, col.b, 1.0);
}

