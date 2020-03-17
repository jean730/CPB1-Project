#include <vector>
#include "client/vertex.h"
#include "client/entity.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <noise/noise.h>
#include <functional>
using namespace std;

float getNoiseValue(noise::module::Perlin &perlin,float x,float y,float z){
	float value = perlin.GetValue(x,y,z);
	if(value<0){
		value/=5;
	}
	std::cout << "perlin: " << value << std::endl;
	return value;
}

Entity createTerrain(int Occ,int Seed, int X, int Z, float magnitude, float noiseZoom, int resolution)
{
	noise::module::Perlin perlin;
	Entity terrain;
	terrain.hasIndexBuffer=true;
	perlin.SetSeed(Seed);
	perlin.SetOctaveCount(Occ);
	int gridX = X;
	int gridZ = Z;
	glm::vec2 uv1;
	glm::vec2 uv2;
	glm::vec2 uv3;
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 v3;
	glm::vec3 dPos1;
	glm::vec3 dPos2;
	glm::vec2 U1;
	glm::vec2 U2;
//	glm::vec3 tangent1;
	float normX;
	float normY;
	float normZ;
	int pp = resolution;
	int terrainSize = 40;
	terrainSize /= pp;
	int t=0;
	for (int x = gridX * (terrainSize)*pp; x <= (gridX * (terrainSize) + terrainSize)*pp; x+=pp) {
	    for (int z = gridZ * (terrainSize)*pp; z <= (gridZ * (terrainSize) + terrainSize)*pp; z+=pp) {
		    v1 = glm::vec3(x, magnitude * getNoiseValue(std::ref(perlin),(float)x * noiseZoom, (float)z * noiseZoom, 0.5), z);         
		    v2 = glm::vec3(x, magnitude * getNoiseValue(std::ref(perlin),(float)x * noiseZoom, ((float)z + pp) * noiseZoom, 0.5), (z + pp));
		    v3 = glm::vec3((x + pp), magnitude * getNoiseValue(std::ref(perlin),((float)x + pp) * noiseZoom, (float)z * noiseZoom, 0.5), z);
		    dPos1 = v2 - v1;
		    dPos2 = v3 - v1;
		    float zoom = 0.25;
		    uv1 = glm::vec2(v1.x,v1.z)*zoom;
		    uv2 = glm::vec2(v2.x,v2.z)*zoom;
		    uv3 = glm::vec2(v3.x,v3.z)*zoom;
		    U1 = uv2-uv1;
		    U2 = uv3-uv1;
		    normX = dPos1.y * dPos2.z - dPos1.z * dPos2.y;
		    normY = dPos1.z * dPos2.x - dPos1.x * dPos2.z;
		    normZ = dPos1.x * dPos2.y - dPos1.y * dPos2.x;
//		    float f = 1.0f/(U1.x*U2.y-U2.x*U1.y);
//		    tangent1.x = f * (U2.y * dPos1.x - U1.y * dPos2.x);
//		    tangent1.y = f * (U2.y * dPos1.y - U1.y * dPos2.y);
//		    tangent1.z = f * (U2.y * dPos1.z - U1.y * dPos2.z);
//		    tangent1 = glm::normalize(tangent1);
		Vertex vertex;
		glm::vec3 vector;
		vector.x = v1.x;
		vector.y = v1.y;
		std::cout << "y=" <<  v1.y << std::endl;
		vector.z = v1.z;
		vertex.Position = vector;
		vector.x = normX;
		vector.y = normY;
		vector.z = normZ;
		vertex.Normal = vector;
//		vector.x = tangent1.x;
//		vector.y = tangent1.y;
//		vector.z = tangent1.z;
//		vertex.Tangent = vector;
		glm::vec2 vec;
		vec.x = uv1.x;
		vec.y = uv1.y;
		vertex.UV = vec;
		terrain.Vertices.push_back(vertex);
	    }
	}
	terrainSize+=1;
	for (int x = 0; x < terrainSize - 1; x++) {
	    for (int z = 0; z < terrainSize - 1; z++) {
		terrain.Indices.push_back(  x    +  z    * terrainSize);
		terrain.Indices.push_back( (x) + (z+1) * terrainSize);
		terrain.Indices.push_back( (x+1) + (z)   * terrainSize);
//
		terrain.Indices.push_back(  (x+1)    +  (z+1)    * terrainSize);
		terrain.Indices.push_back( (x+1)   + (z) * terrainSize);
		terrain.Indices.push_back( (x) + (z+1) * terrainSize);
	    }
	}
	std::cout << "Indices: " << t << std::endl;
	return terrain;
}
