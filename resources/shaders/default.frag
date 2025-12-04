#version 330 core

in vec3 norm;
in vec3 worldPos;
out vec4 fragColor;


uniform int typeList[8];
uniform vec3 posList[8];
uniform vec3 dirList[8];
uniform vec4 colorList[8];
uniform float penumbraList[8];
uniform float angleList[8];
uniform vec3 attenuationList[8];

uniform int lightCount;
uniform float ka;
uniform float ks;
uniform float kd;

uniform vec4 shapeKA;
uniform vec4 shapeKD;
uniform vec4 shapeKS;
uniform float shininess;

uniform vec3 camPos;


void main() {
   vec4 finalColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
   finalColor += shapeKA * ka;
   vec3 normNorm = normalize(norm);
   vec3 dirToCam = normalize(camPos - worldPos);
   float normalDir = dot(normNorm, dirToCam);
   if (normalDir < 0) {
      normNorm = -normNorm;
   }


   for (int i = 0; i < lightCount; i++){
      float distanceFromLight = distance(posList[i], worldPos);
      float attenutation = min(1.0f, 1.0f / (attenuationList[i][0] + attenuationList[i][1] * distanceFromLight +
                                             attenuationList[i][2] * distanceFromLight * distanceFromLight));

      vec3 dirToLight;
      vec4 lightColor = colorList[i];
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
            lightColor = vec4(0.0f,0.0f,0.0f,0.0f);
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

      float diffuseAngle = dot(normNorm, -dirToLight);
      vec4 diffuseColor = shapeKD * kd * attenutation * lightColor * diffuseAngle;
      finalColor += diffuseColor;

      vec3 r = 2 * diffuseAngle * normNorm + dirToLight;
      r = normalize(r);
      float specularAngle = max(0.0f, dot(dirToCam, r));
      vec4 specColor;
      if (shininess == 0.0f){
         specColor = shapeKS * ks * attenutation * lightColor * specularAngle;
      } else{
         specColor = shapeKS * ks * attenutation * lightColor *  pow(specularAngle, shininess);
      }
      finalColor += specColor;
   }
   fragColor = finalColor;
}

