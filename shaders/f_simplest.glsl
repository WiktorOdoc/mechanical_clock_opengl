#version 330


out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela
in vec4 iC;

in vec4 l1;
in vec4 l2;
in vec4 n;
in vec4 v;

uniform sampler2D textureMap0;
uniform sampler2D textureMap1;

uniform bool shaded;

in vec2 iTexCoord0;
in vec2 iTexCoord1;

void main(void) 
{

	if(!shaded) //źródło światła nie ma być ocieniowane
	{
		pixelColor = texture(textureMap0, iTexCoord0);
		return;
	}
	
	vec4 ks = texture(textureMap0, iTexCoord0) * 0.5;
	vec4 kd = mix(texture(textureMap0, iTexCoord0), texture(textureMap1, iTexCoord1), 0.0625);

	vec3 ambient = kd.rgb * vec3(0.25f); //ambient lighting 0.25

	vec4 ml1 = normalize(l1);
	vec4 ml2 = normalize(l2);
	vec4 mn = normalize(n);
	vec4 mv = normalize(v);
	vec4 mr1 = reflect(-ml1,mn); //Wektor odbity
	vec4 mr2 = reflect(-ml2,mn);

	float nl1 = clamp(dot(mn, ml1), 0, 1); //Kosinus kąta pomiędzy wektorami n i l1.
	float nl2 = clamp(dot(mn, ml2), 0, 1); //Kosinus kąta pomiędzy wektorami n i l2.
	float rv1 = pow(clamp(dot(mr1, mv), 0, 1), 25); // Kosinus kąta pomiędzy wektorami r1 i v podniesiony do 25 potęgi
	float rv2 = pow(clamp(dot(mr2, mv), 0, 1), 25); // Kosinus kąta pomiędzy wektorami r2 i v podniesiony do 25 potęgi

	pixelColor = vec4(ambient, kd.a) + vec4(kd.rgb*nl1, kd.a) + vec4(kd.rgb*nl2, kd.a) + vec4(ks.rgb*rv1, ks.a) + vec4(ks.rgb*rv2, ks.a); //suma komponentów na ostateczny kolor piksela;
}
