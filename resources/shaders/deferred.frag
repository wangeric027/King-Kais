#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpec;


uniform int typeList[32];
uniform vec3 posList[32];
uniform vec3 dirList[32];
uniform vec4 colorList[32];
uniform float penumbraList[32];
uniform float angleList[32];
uniform vec3 attenuationList[32];

uniform int lightCount;
uniform float ka;
uniform float ks;
uniform float kd;

uniform vec3 camPos;

void main() {
    vec3 worldPos = texture(gPosition, texCoords).rgb;
    vec3 norm = texture(gNormal, texCoords).rgb;
    vec3 diffuse = texture(gDiffuse, texCoords).rgb;
    vec3 specular = texture(gSpec, texCoords).rgb;
    float shininess = texture(gSpec, texCoords).a;

   vec4 finalColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
   finalColor += vec4(diffuse * 0.1, 1.0f);
   vec3 dirToCam = normalize(camPos - worldPos);

   for (int i = 0; i < lightCount; i++){
      float distanceFromLight = distance(posList[i], worldPos);
      float attenutation = min(1.0f, 1.0f / (attenuationList[i][0] + attenuationList[i][1] * distanceFromLight +
                                             attenuationList[i][2] * distanceFromLight * distanceFromLight));

      vec3 dirToLight;
      vec3 lightColor = colorList[i].rgb;
      switch(typeList[i]){
         case 0: //point
         dirToLight = normalize(worldPos - posList[i]);
         break;
         case 1: //dir
         attenutation = 1.0f;
         dirToLight = normalize(dirList[i]);
         distanceFromLight = 1000.f;
         break;
         case 2: //spot
         dirToLight = normalize(worldPos - posList[i]);
         float angleToLight = acos(dot(dirToLight ,normalize(dirList[i])));
         float outerAngle = angleList[i];
         float innerAngle = outerAngle - penumbraList[i];
         if (angleToLight > outerAngle){
            lightColor = vec3(0.0f,0.0f,0.0f);
            break;
         }
         if (angleToLight <= innerAngle){
            break;
         }

         float falloff = -2 * pow(((angleToLight - innerAngle) / (outerAngle - innerAngle)),3) +
                               3 * pow(((angleToLight - innerAngle) / (outerAngle - innerAngle)),2);
         lightColor = lightColor * (1.0f - falloff);

         break;
      }

      float diffuseAngle = dot(norm, -dirToLight);
      vec4 diffuseColor = vec4(diffuse * kd * attenutation * lightColor * diffuseAngle, 1.0f);
      finalColor += diffuseColor;

      vec3 r = 2 * diffuseAngle * norm + dirToLight;
      r = normalize(r);
      float specularAngle = max(0.0f, dot(dirToCam, r));
      vec4 specColor;
      if (shininess == 0.0f){
         specColor = vec4(specular * ks * attenutation * lightColor * specularAngle, 1.0f);
      } else{
         specColor = vec4(specular * ks * attenutation * lightColor *  pow(specularAngle, shininess), 1.0f);
      }
      finalColor += specColor;
   }
   FragColor = finalColor;
}
