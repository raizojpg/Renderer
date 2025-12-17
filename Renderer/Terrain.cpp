#include "Terrain.h"
#include <cstdlib>
#include <ctime>
#include "SOIL.h"

Terrain::Terrain(int w, int l, int s) : WIDTH{ w }, LENGTH{ l }, NR_VF{ w * l }, step{s} {
	maxHeight = 2550;
	terrainMat = glm::translate(glm::mat4(1.0f), glm::vec3(-(w-1) * s / 2, -(l-1) * s / 2, 2000));
	srand(static_cast<unsigned>(time(0)));

	int width, height, channels;
	heightData = SOIL_load_image("resources/heightmap.bmp", &width, &height, &channels, SOIL_LOAD_L);
	imgWidth = width;
	imgHeight = height;

	patchSize = 33;
	maxLod = 5;

	lodMap = std::vector<std::vector<short>>(WIDTH / (patchSize-1), std::vector<short>(LENGTH / (patchSize-1), 0));
}

void Terrain::CreateVAO(){
	std::vector<glm::vec4> Vertices(NR_VF);
	std::vector<glm::vec3> Colors(NR_VF);
	std::vector<GLushort> Indices(3 * 2 * (patchSize - 1) * (patchSize - 1) * 2);

	int v_idx, t_idx = 0;
	float x, y, z;
	float r, g, b;

	for (int i = 0; i < WIDTH; i++) {
		for (int j = 0; j < LENGTH; j++) {
			v_idx = i * LENGTH + j;
;			x = i * step;
			y = j * step;
			if (heightData && i < imgWidth && j < imgHeight) {
				z = -heightData[i * imgHeight + j] / 255.0f * maxHeight;
			}
			else {
				z = 0.0f; 
			}
			Vertices[v_idx] = glm::vec4(x, y, z, 1.0);
			r = g = b = -z / maxHeight;
			Colors[v_idx] = glm::vec3(r, g, b);
		}
	}

	for(int h=0;h<maxLod;h++){
		int step = 1 << (h + 1);
		LodInfo lod;

		step *= 2; 
		// border for higher lod
		lod.border[1][0].start = t_idx;
		for (int k = 0; k < patchSize - 1; k += step) {
			Indices[3 * t_idx + 0] = k;
			Indices[3 * t_idx + 1] = k + step / 2;
			Indices[3 * t_idx + 2] = (step / 4) * LENGTH + k + step / 4;
			t_idx++;
			Indices[3 * t_idx + 0] = k + step / 2;
			Indices[3 * t_idx + 1] = k + step;
			Indices[3 * t_idx + 2] = (step / 4) * LENGTH + k + 3 * step / 4;
			t_idx++;
		}
		lod.border[1][0].count = t_idx - lod.border[1][0].start;
		lod.border[1][1].start = t_idx;
		for (int k = 0; k < patchSize - 1; k += step) {
			Indices[3 * t_idx + 0] = k * LENGTH + patchSize - 1;
			Indices[3 * t_idx + 1] = (k + step / 2) * LENGTH + patchSize - 1;
			Indices[3 * t_idx + 2] = (k + step / 4) * LENGTH + patchSize - 1 - step / 4;
			t_idx++;
			Indices[3 * t_idx + 0] = (k + step / 2) * LENGTH + patchSize - 1;
			Indices[3 * t_idx + 1] = (k + step) * LENGTH + patchSize - 1;
			Indices[3 * t_idx + 2] = (k + 3 * step / 4) * LENGTH + patchSize - 1 - step / 4;
			t_idx++;
		}
		lod.border[1][1].count = t_idx - lod.border[1][1].start;
		lod.border[1][2].start = t_idx;
		for (int k = 0; k < patchSize - 1; k += step) {
			Indices[3 * t_idx + 0] = (patchSize - 1) * LENGTH + k + step;
			Indices[3 * t_idx + 1] = (patchSize - 1) * LENGTH + k + step / 2;
			Indices[3 * t_idx + 2] = (patchSize - 1 - step / 4) * LENGTH + k + 3 * step / 4;
			t_idx++;
			Indices[3 * t_idx + 0] = (patchSize - 1) * LENGTH + k + step / 2;
			Indices[3 * t_idx + 1] = (patchSize - 1) * LENGTH + k;
			Indices[3 * t_idx + 2] = (patchSize - 1 - step / 4) * LENGTH + k + step / 4;
			t_idx++;
		}
		lod.border[1][2].count = t_idx - lod.border[1][2].start;
		lod.border[1][3].start = t_idx;
		for (int k = 0; k < patchSize - 1; k += step) {
			Indices[3 * t_idx + 0] = (k + step) * LENGTH;
			Indices[3 * t_idx + 1] = (k + step / 2) * LENGTH;
			Indices[3 * t_idx + 2] = (k + 3 * step / 4) * LENGTH + step / 4;
			t_idx++;
			Indices[3 * t_idx + 0] = (k + step / 2) * LENGTH;
			Indices[3 * t_idx + 1] = k * LENGTH;
			Indices[3 * t_idx + 2] = (k + step / 4) * LENGTH + step / 4;
			t_idx++;
		}
		lod.border[1][3].count = t_idx - lod.border[1][3].start;

		step /= 2; 
		//border for current lod
		lod.border[0][0].start = t_idx;
		for (int k = 0; k < patchSize - 1; k += step) {
			Indices[3 * t_idx + 0] = k;
			Indices[3 * t_idx + 1] = k + step / 2;
			Indices[3 * t_idx + 2] = (step / 2) * LENGTH + k + step / 2;
			t_idx++;
			Indices[3 * t_idx + 0] = k + step / 2;
			Indices[3 * t_idx + 1] = k + step;
			Indices[3 * t_idx + 2] = (step / 2) * LENGTH + k + step / 2;
			t_idx++;
		}
		lod.border[0][0].count = t_idx - lod.border[0][0].start;
		lod.border[0][1].start = t_idx;
		for (int k = 0; k < patchSize - 1; k += step) {
			Indices[3 * t_idx + 0] = k * LENGTH + patchSize - 1;
			Indices[3 * t_idx + 1] = (k + step / 2) * LENGTH + patchSize - 1;
			Indices[3 * t_idx + 2] = (k + step / 2) * LENGTH + patchSize - 1 - step / 2;
			t_idx++;
			Indices[3 * t_idx + 0] = (k + step / 2) * LENGTH + patchSize - 1;
			Indices[3 * t_idx + 1] = (k + step) * LENGTH + patchSize - 1;
			Indices[3 * t_idx + 2] = (k + step / 2) * LENGTH + patchSize - 1 - step / 2;
			t_idx++;
		}
		lod.border[0][1].count = t_idx - lod.border[0][1].start;
		lod.border[0][2].start = t_idx;
		for (int k = 0; k < patchSize - 1; k += step) {
			Indices[3 * t_idx + 0] = (patchSize - 1) * LENGTH + k + step;
			Indices[3 * t_idx + 1] = (patchSize - 1) * LENGTH + k + step / 2;
			Indices[3 * t_idx + 2] = (patchSize - 1 - step / 2) * LENGTH + k + step / 2;
			t_idx++;
			Indices[3 * t_idx + 0] = (patchSize - 1) * LENGTH + k + step / 2;
			Indices[3 * t_idx + 1] = (patchSize - 1) * LENGTH + k;
			Indices[3 * t_idx + 2] = (patchSize - 1 - step / 2) * LENGTH + k + step / 2;
			t_idx++;
		}
		lod.border[0][2].count = t_idx - lod.border[0][2].start;
		lod.border[0][3].start = t_idx;
		for (int k = 0; k < patchSize - 1; k += step) {
			Indices[3 * t_idx + 0] = (k + step) * LENGTH;
			Indices[3 * t_idx + 1] = (k + step / 2) * LENGTH;
			Indices[3 * t_idx + 2] = (k + step / 2) * LENGTH + step / 2;
			t_idx++;
			Indices[3 * t_idx + 0] = (k + step / 2) * LENGTH;
			Indices[3 * t_idx + 1] = k * LENGTH;
			Indices[3 * t_idx + 2] = (k + step / 2) * LENGTH + step / 2;
			t_idx++;
		}
		lod.border[0][3].count = t_idx - lod.border[0][3].start;
	
		// center of the patch for the current lod
		lod.center.start = t_idx;
		for (int i = 0; i < patchSize - 1; i += step) {
			for (int j = 0; j < patchSize - 1; j += step) {
				//skipping the border
				if (i != 0) {
					Indices[3 * t_idx + 0] = i * LENGTH + j;
					Indices[3 * t_idx + 1] = i * LENGTH + j + step / 2;
					Indices[3 * t_idx + 2] = (i + step / 2) * LENGTH + j + step / 2;
					t_idx++;
					Indices[3 * t_idx + 0] = i * LENGTH + j + step / 2;
					Indices[3 * t_idx + 1] = i * LENGTH + j + step;
					Indices[3 * t_idx + 2] = (i + step / 2) * LENGTH + j + step / 2;
					t_idx++;
				}
				if (j != patchSize - 1 - step) {
					Indices[3 * t_idx + 0] = i * LENGTH + j + step;
					Indices[3 * t_idx + 1] = (i + step / 2) * LENGTH + j + step;
					Indices[3 * t_idx + 2] = (i + step / 2) * LENGTH + j + step / 2;
					t_idx++;
					Indices[3 * t_idx + 0] = (i + step / 2) * LENGTH + j + step / 2;
					Indices[3 * t_idx + 1] = (i + step / 2) * LENGTH + j + step;
					Indices[3 * t_idx + 2] = (i + step) * LENGTH + j + step;
					t_idx++;
				}
				if (i != patchSize - 1 - step) {
					Indices[3 * t_idx + 0] = (i + step / 2) * LENGTH + j + step / 2;
					Indices[3 * t_idx + 1] = (i + step) * LENGTH + j + step / 2;
					Indices[3 * t_idx + 2] = (i + step) * LENGTH + j;
					t_idx++;
					Indices[3 * t_idx + 0] = (i + step / 2) * LENGTH + j + step / 2;
					Indices[3 * t_idx + 1] = (i + step) * LENGTH + j + step;
					Indices[3 * t_idx + 2] = (i + step) * LENGTH + j + step / 2;
					t_idx++;
				}
				if (j != 0) {
					Indices[3 * t_idx + 0] = (i + step / 2) * LENGTH + j;
					Indices[3 * t_idx + 1] = (i + step / 2) * LENGTH + j + step / 2;
					Indices[3 * t_idx + 2] = (i + step) * LENGTH + j;
					t_idx++;
					Indices[3 * t_idx + 0] = i * LENGTH + j;
					Indices[3 * t_idx + 1] = (i + step / 2) * LENGTH + j + step / 2;
					Indices[3 * t_idx + 2] = (i + step / 2) * LENGTH + j;
					t_idx++;
				}
			}
		}
		lod.center.count = t_idx - lod.center.start;
		
		lods.push_back(lod);
	}

	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);
	glGenBuffers(1, &VboId);
	glGenBuffers(1, &EboId);

	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBufferData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(glm::vec4) + Colors.size() * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, Vertices.size() * sizeof(glm::vec4), Vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, Vertices.size() * sizeof(glm::vec4), Colors.size() * sizeof(glm::vec3), Colors.data());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(GLushort), Indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(Vertices.size() * sizeof(glm::vec4)));

}

void Terrain::Draw(){
	this->Bind();
	for (int i = 0; i < WIDTH - 1; i += (patchSize - 1)) {
		for (int j = 0; j < LENGTH - 1; j += (patchSize - 1)) {
			int baseVertex = i * (LENGTH) + j;

			int ii = i / (patchSize - 1);
			int jj = j / (patchSize - 1);
			int lod = lodMap[ii][jj];

			int nlod = (ii - 1 >= 0) ? lodMap[ii - 1][jj] : lod;
			nlod = (nlod > lod) ? nlod - lod : 0;
			glDrawElementsBaseVertex(GL_TRIANGLES, 3 * lods[lod].border[nlod][0].count, GL_UNSIGNED_SHORT, (GLvoid*)(3 * lods[lod].border[nlod][0].start * sizeof(GLushort)), baseVertex);
			
			int elod = (jj + 1 < LENGTH / (patchSize-1)) ? lodMap[ii][jj + 1] : lod;
			elod = (elod > lod) ? elod - lod : 0;
			glDrawElementsBaseVertex(GL_TRIANGLES, 3 * lods[lod].border[elod][1].count, GL_UNSIGNED_SHORT, (GLvoid*)(3 * lods[lod].border[elod][1].start * sizeof(GLushort)), baseVertex);
			
			int slod = (ii + 1 < WIDTH / (patchSize-1)) ? lodMap[ii + 1][jj] : lod;
			slod = (slod > lod) ? slod - lod : 0;
			glDrawElementsBaseVertex(GL_TRIANGLES, 3 * lods[lod].border[slod][2].count, GL_UNSIGNED_SHORT, (GLvoid*)(3 * lods[lod].border[slod][2].start * sizeof(GLushort)), baseVertex);
			
			int wlod = (jj - 1 >= 0) ? lodMap[ii][jj - 1] : lod;
			wlod = (wlod > lod) ? wlod - lod : 0;
			glDrawElementsBaseVertex(GL_TRIANGLES, 3 * lods[lod].border[wlod][3].count, GL_UNSIGNED_SHORT, (GLvoid*)(3 * lods[lod].border[wlod][3].start * sizeof(GLushort)), baseVertex);

			glDrawElementsBaseVertex(GL_TRIANGLES, 3 * lods[lod].center.count, GL_UNSIGNED_SHORT, (GLvoid*)(3 * lods[lod].center.start * sizeof(GLushort)), baseVertex);
		}
	}
}

int Terrain::getWidth() { return WIDTH; }

int Terrain::getLength() { return LENGTH; }

glm::mat4 Terrain::getTerrainMat() { return terrainMat; }

void Terrain::updateLodMap(glm::vec3 observer){
	glm::vec4 obs = glm::vec4(observer, 0) - terrainMat[3];
	float x = obs.x;
	float y = obs.y;
	float z = obs.z;
	
	for (int i = 0; i < WIDTH - 1; i += (patchSize - 1)) {
		for (int j = 0; j < LENGTH - 1; j += (patchSize - 1)) {
			float midi = (i + patchSize / 2) * step;
			float midj = (j + patchSize / 2) * step;
			float distance = glm::sqrt((midi - x) * (midi - x) + (midj - y) * (midj - y));
			short lod = std::min(int(distance) / step / patchSize / 2, maxLod - 1);
			lodMap[int(i / (patchSize - 1))][int(j / (patchSize - 1))] = lod;
			//std::cout << lod << " ";
		}
		//std::cout << std::endl;
	}

}

Terrain::~Terrain(){
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	if (heightData) {
		SOIL_free_image_data(heightData);
		heightData = nullptr;
	}
}
