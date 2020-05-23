#include "Texture.hpp"

#include <SOIL2.h>

#include <vector>
#include <iostream>

namespace
{
    /**
    Библиотека SOIL читает текстуры перевернутыми
    */
    void invertY(unsigned char* image, int width, int height, int channels)
    {
        for (int j = 0; j * 2 < height; ++j)
        {
            unsigned int index1 = j * width * channels;
            unsigned int index2 = (height - 1 - j) * width * channels;
            for (int i = 0; i < width * channels; i++)
            {
                unsigned char temp = image[index1];
                image[index1] = image[index2];
                image[index2] = temp;
                ++index1;
                ++index2;
            }
        }
    }
}

void Texture::saveRGBA8_PNG(const char *filename) {
    int packAlignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &packAlignment);

    int width, height;
    getSize(width, height);
    std::vector<unsigned char> pixels(4 * size_t(width) * size_t(height));

    // Reset pack alignment: no alignment.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    glPixelStorei(GL_PACK_ALIGNMENT, packAlignment);

    invertY(pixels.data(), width, height, 4);

    SOIL_save_image(filename, SOIL_SAVE_TYPE_PNG, width, height, 4, pixels.data());
    std::cout << "save image to " << filename << ": " << SOIL_last_result() << std::endl;
}

TexturePtr loadTextureGL(const std::string& filename) {
    // If we require SOIL mipmap flag, it will make the texture of quad shape.
    GLuint texId = SOIL_load_OGL_texture(filename.data(), 0, 0, SOIL_FLAG_INVERT_Y);
    std::cout << SOIL_last_result() << std::endl;
    TexturePtr texture = std::make_shared<Texture>(texId, GL_TEXTURE_2D);
    texture->generateMipmaps();
    return texture;
}

TexturePtr loadTexture(const std::string& filename, SRGB srgb, bool prefer1D)
{
    int width, height, channels;
    unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);
    if (!image)
    {
        std::cerr << "SOIL loading error: " << SOIL_last_result() << std::endl;
        return std::make_shared<Texture>();
    }

    invertY(image, width, height, channels);

    GLint internalFormat;
    if (srgb == SRGB::YES)
    {
        internalFormat = (channels == 4) ? GL_SRGB8 : GL_SRGB8_ALPHA8;
    }
    else
    {
        internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;
    }

    GLint format = (channels == 4) ? GL_RGBA : GL_RGB;

	bool is1DTexture = (height == 1);
	GLenum target = (prefer1D && is1DTexture) ? GL_TEXTURE_1D : GL_TEXTURE_2D;

    TexturePtr texture = std::make_shared<Texture>(target);
	if (is1DTexture) {
		texture->setTexImage1D(target, 0, internalFormat, width, format, GL_UNSIGNED_BYTE, image);
	}
	else {
		texture->setTexImage2D(target, 0, internalFormat, width, height, format, GL_UNSIGNED_BYTE, image);
	}
    texture->generateMipmaps();

    SOIL_free_image_data(image);

    return texture;
}

TexturePtr loadTextureDDS(const std::string& filename)
{
    GLuint tex = SOIL_load_OGL_texture(filename.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_DDS_LOAD_DIRECT);
    if (tex == 0)
    {
        std::cerr << "SOIL loading error: " << SOIL_last_result() << std::endl;
        return std::make_shared<Texture>();
    }

    return std::make_shared<Texture>(tex, GL_TEXTURE_2D);
}

//==========================================================

namespace
{
    //Удобная функция для вычисления цвета из линейной палитры от синего до красного
    glm::vec3 getColorFromLinearPalette(float value)
    {
        if (value < 0.25f)
        {
            return glm::vec3(0.0f, value * 4.0f, 1.0f);
        }
        else if (value < 0.5f)
        {
            return glm::vec3(0.0f, 1.0f, (0.5f - value) * 4.0f);
        }
        else if (value < 0.75f)
        {
            return glm::vec3((value - 0.5f) * 4.0f, 1.0f, 0.0f);
        }
        else
        {
            return glm::vec3(1.0f, (1.0f - value) * 4.0f, 0.0f);
        }
    }
}

TexturePtr makeProceduralTexture()
{
    int width = 128;
    int height = 128;

    std::vector<unsigned char> data;

    for (int row = 0; row < height; row++)
    {
        for (int column = 0; column < width; column++)
        {
            glm::vec3 color = getColorFromLinearPalette((float)column / width);

            data.push_back(static_cast<unsigned char>(255 * color.r));
            data.push_back(static_cast<unsigned char>(255 * color.g));
            data.push_back(static_cast<unsigned char>(255 * color.b));
        }
    }

    TexturePtr texture = std::make_shared<Texture>(GL_TEXTURE_2D);
    texture->setTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, GL_RGB, GL_UNSIGNED_BYTE, data.data());

    return texture;
}

//==========================================================

namespace
{
    void loadCubeTextureFace(const TexturePtr& texture, GLenum target, const std::string& filename)
    {
        int width, height, channels;
        unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, &channels, SOIL_LOAD_RGB);
        if (!image)
        {
            std::cerr << "SOIL loading error: " << SOIL_last_result() << std::endl;
            return;
        }

        texture->setTexImage2D(target, 0, GL_RGB8, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);

        SOIL_free_image_data(image);
    }
}

TexturePtr loadCubeTexture(const std::string& basefilename)
{
    TexturePtr texture = std::make_shared<Texture>(GL_TEXTURE_CUBE_MAP);

    loadCubeTextureFace(texture, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, basefilename + "/negx.jpg");
    loadCubeTextureFace(texture, GL_TEXTURE_CUBE_MAP_POSITIVE_X, basefilename + "/posx.jpg");
    loadCubeTextureFace(texture, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, basefilename + "/negy.jpg");
    loadCubeTextureFace(texture, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, basefilename + "/posy.jpg");
    loadCubeTextureFace(texture, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, basefilename + "/negz.jpg");
    loadCubeTextureFace(texture, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, basefilename + "/posz.jpg");

    texture->generateMipmaps();

    return texture;
}