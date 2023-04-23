#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "PathGraphInterface.hpp"
#include "Bitmap.hpp"

using namespace GraphGenerator;
using namespace std::literals::string_literals;

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cerr << "Insufficient arguments! <OFF input> <layer> [-p <Path output>] [-b <Bitmap output>]" << std::endl;
		return -1;
	}
	auto create_path = false;
	auto path_output = ""s;
	auto create_bitmap = false;
	auto bitmap_output = ""s;
	auto rotate = false;
	for (auto i = 3; i < argc; i++)
	{
		if (std::string(argv[i]) == "-p")
		{
			if (i + 1 >= argc)
			{
				std::cerr << "Insufficient arguments! <OFF input> <layer> [-p <Path output>] [-b <Bitmap output>]" << std::endl;
				return -1;
			}
			else
			{
				create_path = true;
				path_output = argv[i + 1];
			}
		}
		if (std::string(argv[i]) == "-b")
		{
			if (i + 1 >= argc)
			{
				std::cerr << "Insufficient arguments! <OFF input> <layer> [-p <Path output>] [-b <Bitmap output>]" << std::endl;
				return -1;
			}
			else
			{
				create_bitmap = true;
				bitmap_output = argv[i + 1];
			}
		}

		if (std::string(argv[i]) == "-r")
		{
			rotate = true;
		}
	}

	std::cout << "Parsing " << argv[1] << std::endl;
	auto file = std::ifstream(argv[1]);
	std::string head;
	file >> head;
	if (!head.starts_with("OFF"))
	{
		file.close();
		std::cerr << "Not an OFF file!" << std::endl;
		return -1;
	}
	auto vertexCount = 0;
	auto faceCount = 0;
	auto edgeCount = 0;
	// Normal case
	if (head == "OFF")
	{
		file >> vertexCount >> faceCount >> edgeCount;
	}
	else
	{
		vertexCount = std::stoi(head.substr(3));
		file >> faceCount >> edgeCount;
	}
	std::vector<Vector3> vertexList(vertexCount);
	auto minX = std::numeric_limits<float>::max();
	auto minY = std::numeric_limits<float>::max();
	auto minZ = std::numeric_limits<float>::max();
	auto maxX = std::numeric_limits<float>::min();
	auto maxY = std::numeric_limits<float>::min();
	auto maxZ = std::numeric_limits<float>::min();
	for (auto i = 0; i < vertexCount; i++)
	{
		file >> vertexList[i].x >> vertexList[i].y >> vertexList[i].z;
		minX = vertexList[i].x < minX ? vertexList[i].x : minX;
		minY = vertexList[i].y < minY ? vertexList[i].y : minY;
		minZ = vertexList[i].z < minZ ? vertexList[i].z : minZ;
		maxX = vertexList[i].x > maxX ? vertexList[i].x : maxX;
		maxY = vertexList[i].y > maxY ? vertexList[i].y : maxY;
		maxZ = vertexList[i].z > maxZ ? vertexList[i].z : maxZ;
	}
	for (auto i = 0; i < vertexCount; i++)
	{
		auto scale = std::max(maxX - minX, std::max(maxY - minY, maxZ - minZ));
		vertexList[i].x = (vertexList[i].x - minX) / scale;
		vertexList[i].y = (vertexList[i].y - minY) / scale;
		vertexList[i].z = (vertexList[i].z - minZ) / scale;
		if (vertexList[i].x < 0)
		{
			file.close();
			std::cerr << "Vertex position x < 0! Vertex id = " << i << " value = " << vertexList[i].x << std::endl;
			return -1;
		}
		if (vertexList[i].y < 0)
		{
			file.close();
			std::cerr << "Vertex position y < 0! Vertex id = " << i << " value = " << vertexList[i].y << std::endl;
			return -1;
		}
		if (vertexList[i].z < 0)
		{
			file.close();
			std::cerr << "Vertex position z < 0! Vertex id = " << i << " value = " << vertexList[i].z << std::endl;
			return -1;
		}
	}

	auto graph = makePathGraphWithMemoryPool(1, 0, 1);
	auto layer = std::atoi(argv[2]);
	for (auto i = 0; i < faceCount; i++)
	{
		auto shape = 0;
		auto point1 = 0;
		auto point2 = 0;
		auto point3 = 0;
		file >> shape >> point1 >> point2 >> point3;
		if (shape != 3)
		{
			file.close();
			std::cerr << "Not a triangle! Vertex id = " << i << " value = " << shape << std::endl;
			return -1;
		}
		graph->addTerrainTriangleMesh(vertexList[point1], vertexList[point2], vertexList[point3], layer, false);
	}
	file.close();

	graph->calculateTerrainPathGraph();
	auto componentIndex = graph->getComponentTotalCount();

	std::cout << "Total component count = " << componentIndex << std::endl;
	auto maxSize = 0;
	auto maxIndex = 0;
	for (auto i = 1; i <= componentIndex; i++)
	{
		std::cout << "Component " << i << " has size " << graph->getComponentSize(i) << std::endl;
		auto component = graph->getComponentGraph(i, rotate);
		std::cout << "Vertex = " << component.first.size() << " Edge = " << component.second.size() << std::endl;
		if (component.first.size() > maxSize)
		{
			maxSize = component.first.size();
			maxIndex = i;
		}
	}
	
	if (maxIndex == 0)
	{
		std::cerr << "Cannot find valid component" << std::endl;
		destroyPathGraph(graph);
		return -1;
	}

	auto result = graph->getComponentGraph(maxIndex, rotate);
	if (create_path)
	{
		auto output = std::ofstream(path_output);
		output << "PATHGRAPH" << std::endl;
		output << result.first.size() << " " << result.second.size() << std::endl;
		for (auto& position : result.first)
		{
			output << position.x << " " << position.y << " " << position.z << std::endl;
		}
		for (auto& link : result.second)
		{
			output << link.first << " " << link.second << std::endl;
		}
		output.close();
	}

	if (create_bitmap)
	{
		auto output = std::ofstream(bitmap_output, std::ios::binary);
		writeToFiles(graph->getComponentColorGraph(maxIndex, layer), output);
		output.close();
	}
	
	destroyPathGraph(graph);
	return 0;
}