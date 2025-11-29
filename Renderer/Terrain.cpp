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
}

void Terrain::CreateVAO(){
	std::vector<glm::vec4> Vertices(NR_VF);
	std::vector<glm::vec3> Colors(NR_VF);
	std::vector<GLushort> Indices(3 * 2 * (NR_VF - 2));

	int v_idx, i_idx = 0;
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
			
			if (i < WIDTH - 1 && j < LENGTH - 1) {
				Indices[3 * i_idx] = v_idx;
				Indices[3 * i_idx + 1] = v_idx + 1;
				Indices[3 * i_idx + 2] = (i + 1) * LENGTH + j + 1;
				i_idx++;

				Indices[3 * i_idx] = v_idx;
				Indices[3 * i_idx + 1] = (i + 1) * LENGTH + j + 1;
				Indices[3 * i_idx + 2] = (i + 1) * LENGTH + j;
				i_idx++;
			}
		}
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
	for (int i_idx = 0; i_idx < 2 * NR_VF - 2; i_idx++){
			glDrawElements(
				GL_TRIANGLES,3,GL_UNSIGNED_SHORT,
				(GLvoid*)((3 * i_idx) * sizeof(GLushort)));
	}
}

int Terrain::getWidth() { return WIDTH; }

int Terrain::getLength() { return LENGTH; }

glm::mat4 Terrain::getTerrainMat() { return terrainMat; }

Terrain::~Terrain(){
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	if (heightData) {
		SOIL_free_image_data(heightData);
		heightData = nullptr;
	}
}
