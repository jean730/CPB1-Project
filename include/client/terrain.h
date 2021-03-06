#ifndef TERRAIN_H
#define TERRAIN_H
#include <noise/noise.h>
float getNoiseValue(noise::module::Perlin &perlin,float x,float y,float z);
Entity createTerrain(int Occ,int Seed, int X, int Z, float magnitude, float noiseZoom, int resolution);
float getTerrainHeight(int Occ,int Seed, float x, float z, float magnitude, float noiseZoom);
#endif
