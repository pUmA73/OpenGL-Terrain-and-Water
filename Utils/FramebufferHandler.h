#ifndef FRAMEBUFFERHANDLER_H
#define FRAMEBUFFERHANDLER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class FrameBufferHandler {
public:
	static const int REFLECTION_WIDTH = 320;
	static const int REFLECTION_HEIGHT = 180;

	static const int REFRACTION_WIDTH = 1280;
	static const int REFRACTION_HEIGHT = 720;

	static const int screenWidth = 1600;
	static const int screenHeight = 1200;

	FrameBufferHandler(int startingTexSlot = 0) : textureStartSlot(startingTexSlot)
	{
		initializeReflectionFrameBuffer();
		initializeRefractionFrameBuffer();
	}

	~FrameBufferHandler()
	{
		cleanUp();
	}

	void cleanUp()
	{
		glDeleteFramebuffers(1, &reflectionFrameBuffer);
		glDeleteTextures(1, &reflectionTexture);
		glDeleteRenderbuffers(1, &reflectionDepthBuffer);

		glDeleteFramebuffers(1, &refractionFrameBuffer);
		glDeleteTextures(1, &refractionTexture);
		glDeleteTextures(1, &refractionDepthTexture);
	}

	void bindReflectionFrameBuffer()
	{
		bindFrameBuffer(reflectionFrameBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
	}

	void bindRefractionFrameBuffer()
	{
		bindFrameBuffer(refractionFrameBuffer, REFRACTION_WIDTH, REFRACTION_HEIGHT);
	}

	void unbindCurrentFrameBuffer(int screenWidth, int screenHeight)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);
	}

	GLuint getReflectionTexture() const
	{
		return reflectionTexture;
	}

	GLuint getRefractionTexture() const
	{
		return refractionTexture;
	}

	GLuint getRefractionDepthTexture() const
	{
		return refractionDepthTexture;
	}

private:
	GLuint reflectionFrameBuffer = 0;
	GLuint reflectionTexture = 0;
	GLuint reflectionDepthBuffer = 0;

	GLuint refractionFrameBuffer = 0;
	GLuint refractionTexture = 0;
	GLuint refractionDepthTexture = 0;

	int textureStartSlot;

	GLuint createFrameBuffer()
	{
		GLuint frameBuffer;
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		return frameBuffer;
	}

	void bindFrameBuffer(GLuint frameBuffer, int width, int height)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, width, height);
	}

	GLuint createTextureAttachment(int width, int height, int textureUnit)
	{
		GLuint texture;
		glGenTextures(1, &texture);

		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		return texture;
	}

	GLuint createDepthTextureAttachment(int width, int height, int textureUnit)
	{
		GLuint texture;
		glGenTextures(1, &texture);

		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);

		return texture;
	}

	GLuint createDepthBufferAttachment(int width, int height)
	{
		GLuint depthBuffer;
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		return depthBuffer;
	}

	void initializeReflectionFrameBuffer()
	{
		reflectionFrameBuffer = createFrameBuffer();
		reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT, textureStartSlot);
		reflectionDepthBuffer = createDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
		unbindCurrentFrameBuffer(screenWidth, screenHeight);
		
	}

	void initializeRefractionFrameBuffer()
	{
		refractionFrameBuffer = createFrameBuffer();
		refractionTexture = createTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT, textureStartSlot + 1);
		refractionDepthTexture = createDepthTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT, textureStartSlot + 2);
		unbindCurrentFrameBuffer(screenWidth, screenHeight);
	}
};

#endif	// FRAMEBUFFERHANDLER_H
