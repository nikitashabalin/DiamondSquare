#include <iostream>
#include <string>
#include <math.h>
#include <random>
#include <vector>
#include <SFML/Graphics.hpp>
#include "main.h"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include <chrono>

struct vector2d {
    float x, y;
};

struct vector3d {
    float x, y, z;
};

struct HeightAndGradient {
    float height;
    float gradientX;
    float gradientY;
};

struct Particle {
    //Construct Particle at Position
    Particle(vector2d _pos) { pos = _pos; }

    vector2d pos;
    vector2d speed = { 0.0, 0.0 };

    float volume = 1.0;   //This will vary in time
    float sediment = 0.0; //Fraction of Volume that is Sediment!
};

float vectorLength(vector2d speed) {
    return sqrt(speed.x * speed.x + speed.y * speed.y);
}

vector3d normalizeVector(vector3d vec) {
    float len = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);

    if (len != 0) {
        vec.x /= len;
        vec.y /= len;
        vec.z /= len;
    }

    return vec;
}

vector3d getNormal(float curX, float curY, std::vector<std::vector<float>>& map) {

    float scale = 60.0;

    vector3d vec1 = { (map[curX][curY] - map[curX + 1][curY]) * scale, 1.0, 0.0 };    //Positive X
    vector3d vec2 = { (map[curX - 1][curY] - map[curX][curY]) * scale, 1.0, 0.0 };    //Negative X
    vec1 = normalizeVector(vec1);
    vec2 = normalizeVector(vec2);

    vector3d vec3 = { 0.0, 1.0, (map[curX][curY] - map[curX][curY + 1]) * scale };    //Positive Y
    vector3d vec4 = { 0.0, 1.0, (map[curX][curY - 1] - map[curX][curY]) * scale };    //Negative Y
    vec3 = normalizeVector(vec3);
    vec4 = normalizeVector(vec4);

    vector3d n;
    n.x = vec1.x * 0.15; n.y = vec1.y * 0.15; n.z = vec1.z * 0.15;
    n.x += vec2.x * 0.15; n.y += vec2.y * 0.15; n.z += vec2.z * 0.15;
    n.x += vec3.x * 0.15; n.y += vec3.y * 0.15; n.z += vec3.z * 0.15;
    n.x += vec4.x * 0.15; n.y += vec4.y * 0.15; n.z += vec4.z * 0.15;

    //-----------------------------------------FOR DIAGONALS-----------------------------------------------//
    vector3d vecD1 = { scale * (map[curX][curY] - map[curX + 1][curY + 1]) / sqrt(2), sqrt(2), scale * (map[curX][curY] - map[curX + 1][curY + 1]) / sqrt(2) };     //Positive Y
    vector3d vecD2 = { scale * (map[curX][curY] - map[curX + 1][curY - 1]) / sqrt(2), sqrt(2), scale * (map[curX][curY] - map[curX + 1][curY - 1]) / sqrt(2) };     //Positive Y
    vector3d vecD3 = { scale * (map[curX][curY] - map[curX - 1][curY + 1]) / sqrt(2), sqrt(2), scale * (map[curX][curY] - map[curX - 1][curY + 1]) / sqrt(2) };     //Positive Y
    vector3d vecD4 = { scale * (map[curX][curY] - map[curX - 1][curY - 1]) / sqrt(2), sqrt(2), scale * (map[curX][curY] - map[curX - 1][curY - 1]) / sqrt(2) };     //Positive Y

    vecD1 = normalizeVector(vecD1);
    vecD2 = normalizeVector(vecD2);
    vecD3 = normalizeVector(vecD3);
    vecD4 = normalizeVector(vecD4);

    n.x += vecD1.x * 0.1; n.y += vecD1.y * 0.1; n.z += vecD1.z * 0.1;
    n.x += vecD2.x * 0.1; n.y += vecD2.y * 0.1; n.z += vecD2.z * 0.1;
    n.x += vecD3.x * 0.1; n.y += vecD3.y * 0.1; n.z += vecD3.z * 0.1;
    n.x += vecD4.x * 0.1; n.y += vecD4.y * 0.1; n.z += vecD4.z * 0.1;

    return n;
}

int randomize(int R) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-R, R);
    return dis(gen);
}

void square_step(std::vector<std::vector<float>>& heightMap, int hmSize, int chSize, int half, int R)
{
    for (int y = 0; y < hmSize - 1; y += chSize) {
        for (int x = 0; x < hmSize - 1; x += chSize) {
            heightMap[y + half][x + half] = ((heightMap[y][x] + heightMap[y][x + chSize] + heightMap[y + chSize][x] + heightMap[y + chSize][x + chSize]) / 4.0) + randomize(R);
        }
    }
}

void diamond_step(std::vector<std::vector<float>>& heightMap, int hmSize, int chSize, int half, int R)
{
    int sum = 0;
    int count = 0;
    for (int y = 0; y < hmSize; y += half) {

        for (int x = (y + half) % chSize; x < hmSize; x += chSize) {
            if ((y - half) >= 0) {
                count++;
                sum += heightMap[y - half][x];
            }

            if ((y + half) <= hmSize - 1) {
                count++;
                sum += heightMap[y + half][x];
            }

            if ((x - half) >= 0) {
                count++;
                sum += heightMap[y][x - half];
            }
            if ((x + half) <= hmSize - 1) {
                count++;
                sum += heightMap[y][x + half];
            }
            heightMap[y][x] = sum / count + randomize(R);
            sum = 0;
            count = 0;
        }
    }
}


int main()
{
    int R = 255;
    int R1 = 0, R2 = 255;
    int heightMapSize = 1024;
    std::cout << "Enter map size: ";
    std::cin >> heightMapSize;
    std::cout << "Enter range [R1;R2]:" << std::endl;
    std::cout << "Enter R1: ";
    std::cin >> R1; 
    std::cout << "Enter R2: ";
    std::cin >> R2;

    std::cout << "Enter R: ";
    std::cin >> R;

    int chunkSize = heightMapSize - 1;
    std::vector<std::vector<float>> heightMap(heightMapSize, std::vector<float>(heightMapSize, 0));
    //----------------------------------------------
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(R1, R2);

    heightMap[0][0] = dis(gen);
    heightMap[chunkSize][0] = dis(gen);
    heightMap[chunkSize][chunkSize] = dis(gen);
    heightMap[0][chunkSize] = dis(gen);

    while (chunkSize > 1)
    {
        int half = chunkSize / 2;
        square_step(heightMap, heightMapSize, chunkSize, half, R);
        diamond_step(heightMap, heightMapSize, chunkSize, half, R);

        chunkSize /= 2;
        R /= 2;
    }

    bool a = 0;
    int iterationsNum = 200000;
    std::cout << "Erosion? " << "1 / 0" << std::endl;
    std::cin >> a;
    

    if (a) {
        std::cout << "Number of iterations: ";
        std::cin >> iterationsNum;
        float minVol = 0.01;
        float density = 1.0;
        float friction = 0.05;
        float depositionRate = 0.1;
        float evapRate = 0.001;
        float dt = 1.4;

        for (int iteration = 0; iteration < iterationsNum; ++iteration) {

            vector2d newpos;
            newpos.x = rand() % (heightMapSize - 1);
            newpos.y = rand() % (heightMapSize - 1);

            Particle drop(newpos);


            while (drop.volume > minVol) {

                vector2d ipos = drop.pos;

                if (ipos.x + 1 < 0 || ipos.x - 1 < 0 || ipos.x + 1 >= heightMapSize - 1 || ipos.y - 1 >= heightMapSize - 1 || ipos.y + 1 < 0 || ipos.y - 1 < 0 || ipos.y + 1 >= heightMapSize - 1 || ipos.y - 1 >= heightMapSize - 1) {
                    break;
                }
                vector3d n = getNormal(ipos.x, ipos.y, heightMap);

                vector2d force = { n.x, n.z };
                drop.speed.x += force.x * dt / (drop.volume * density);
                drop.speed.y += force.y * dt / (drop.volume * density);

                drop.pos.x += dt * drop.speed.x;
                drop.pos.y += dt * drop.speed.y;

                drop.speed.x *= (1.0 - dt * friction);
                drop.speed.y *= (1.0 - dt * friction);

                if (drop.pos.x < 0 || drop.pos.x >= heightMapSize - 1 || drop.pos.y < 0 || drop.pos.y >= heightMapSize - 1) {
                    break;
                }

                float maxSediment = drop.volume * vectorLength(drop.speed) * (heightMap[ipos.x][ipos.y] - heightMap[(int)drop.pos.x][(int)drop.pos.y]);
                if (maxSediment < 0.0) maxSediment = 0.0;
                float sDiff = maxSediment - drop.sediment;

                drop.sediment += dt * depositionRate * sDiff;
                heightMap[ipos.x][ipos.y] -= dt * drop.volume * depositionRate * sDiff;
                drop.volume *= (1.0 - dt * evapRate);

            }
        }
    }

    std::string fileName;
    std::cout << "Enter file name" << std::endl;
    std::cin >> fileName;
    fileName = fileName + ".png";

    sf::Uint8* pixels = new sf::Uint8[heightMapSize * heightMapSize * 4];

    for (int y = 0; y < heightMapSize; ++y) {
        for (int x = 0; x < heightMapSize; ++x) {
            int index = (y * heightMapSize + x) * 4;
            int color = heightMap[x][y];

            // Clipping
            color = (color > 255) ? 255 : ((color < 0) ? 0 : color);

            pixels[index] = color;
            pixels[index + 1] = color;
            pixels[index + 2] = color;
            pixels[index + 3] = 255;
        }

    }

    sf::Texture texture;
    sf::Sprite sprite;
    sf::Sprite spriteForSave;

    texture.create(heightMapSize, heightMapSize);

    texture.update(pixels);

    sprite.setTexture(texture);

    sf::Image image1;
    image1 = sprite.getTexture()->copyToImage();
    image1.saveToFile("Perlin8bit.png");

    cv::Mat image = cv::imread("Perlin8bit.png", cv::IMREAD_GRAYSCALE);
    cv::Mat scaledImage;
    image.convertTo(scaledImage, CV_16U, 256); 
    cv::resize(scaledImage, scaledImage, cv::Size(heightMapSize - 1, heightMapSize - 1), 0, 0, cv::INTER_LINEAR); 
    cv::Mat blurredImage;
    cv::GaussianBlur(scaledImage, blurredImage, cv::Size(15, 15), 0);
    cv::imwrite(fileName, blurredImage);
    cv::imshow("16-bit Image from Array", blurredImage);
    cv::waitKey(0);

    return 0;
}