#define CL_HPP_TARGET_OPENCL_VERSION 210
#include <CL/cl2.hpp>
#include <iostream>
#include <fstream>
#include <gl/glad.h>
#include <gl/glfw3.h>
#include <CL/cl_gl.h>
#include <gl/GL.h>
#include <Windows.h>

typedef struct {
	double red;
	double green;
	double blue;
}RGB;

double clamp(double x, double min, double max) {

	if (x < min) return min;
	if (x > max) return max;

	return x;
}

RGB* colorTemperatureToRGB(double kelvin) {
	
	double temp = kelvin / 100;
	RGB* colors = new RGB{};

	if (temp <= 66) {

		colors->red = 255;
		colors->green = temp;
		colors->green = 99.4708025861 * log(colors->green) - 161.1195681661;

		if (temp <= 19) {
			colors->blue = 0;
		}
		else {
			colors->blue = temp - 10;
			colors->blue = 138.5177312231 * log(colors->blue) - 305.0447927307;
		}
	}
	else {
		colors->red = temp - 60;
		colors->red = 329.698727446 * pow(colors->red, -0.1332047592);

		colors->green = temp - 60;
		colors->green = 288.1221695283 * pow(colors->green, -0.0755148492);

		colors->blue = 255;
	}

	colors->red = clamp(colors->red, 0, 255);
	colors->green = clamp(colors->green, 0, 255);
	colors->blue = clamp(colors->blue, 0, 255);

	return colors;
}



int main() {

	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	cl::Platform platform = platforms[0];
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
	cl::Device device = devices[0];
	cl::Program::Sources sources;

	cl_context_properties props[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(),
		0
	};
	cl::Context context(device, props);

	std::string kernel_code = "		__kernel void MetalPlate(__global double* vector, __global double* copy_vector, int M, int N, int x0, int y0) {"
		"	int y = get_global_id(0);"
		"	int x = get_global_id(1);"
		"	if (x == 0 || x == (M - 1) || y == 0 || y == N - 1 || (x == (x0 + 1) && y == (y0 + 1))) {"
		"		return;}"
		"	double mean = 0;"
		"	for (int i = -1; i <= 1; i++) {"
		"		for (int j = -1; j <= 1; j++) {"
		"			mean += copy_vector[(x + i) * N + y + j];}}"
		"	vector[x * N + y] = mean / 9;}";

	sources.push_back({ kernel_code.c_str(), kernel_code.length() });
	cl::Program program(context, sources);
	if (program.build({ device }) != CL_SUCCESS) {
		std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
		exit(1);
	}

	int M, N, x0, y0;
	double Ts, Tr;

	puts("Provide M, N, x0, y0:");
	std::cin >> M >> N >> x0 >> y0;
	puts("Provide Tr and Ts:");
	std::cin >> Tr >> Ts;

	M += 2;
	N += 2;


	/*GLfloat points[] = { -0.5,-0.5,0.5,-0.5,0.5,0.5,-0.5,0.5,-0.5,-0.5 };
	GLfloat nPoints = 5;
	
	GLFWwindow* window;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(800, 600, "MetalPlate OpenGL Application", NULL, NULL);
	glfwMakeContextCurrent(window);

	if (!window) {
		std::cout << "Failed to create window!" << std::endl;
		glfwTerminate();
		return -1;
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
		});
	

	GLuint vbo;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_LINE_STRIP, 0, nPoints);

		glfwSwapBuffers(window);
		double t(glfwGetTime());
		points[0] = cos(t) * 0.5; points[1] = sin(t) * 0.5;
		points[2] = cos(t + 1.57) * 0.5; points[3] = sin(t + 1.57) * 0.5;
		points[4] = cos(t + 3.14) * 0.5; points[5] = sin(t + 3.14) * 0.5;
		points[6] = cos(t + 4.71) * 0.5; points[7] = sin(t + 4.71) * 0.5;
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glfwPollEvents();
	}

	glfwTerminate();
	
	*/
	double* vector = new double[M * N];
	double* copy_vector = new double[M * N];

	for (int i = 0; i < M * N; i++) {
		vector[i] = Tr;
		copy_vector[i] = Tr;
	}
	puts("\n---Every Matrix has 2 extra rows and columns for the ambient temperature.--- \n");
	std::cout << '\n' << "--------------------------P(0)--------------------------"<< '\n';
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			std::cout << vector[i * N + j] << " ";
			copy_vector[i * N + j] = vector[i * N + j];
		}
		std::cout << '\n';
	}

	vector[(x0 + 1) * N + y0 + 1] = Ts;
	copy_vector[(x0 + 1) * N + y0 + 1] = Ts;
	int count = 1;
	while (true) {
		
		cl::Buffer buf(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, M * N * sizeof(double), vector);
		cl::Buffer copyBuf(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, M * N * sizeof(double), copy_vector);
		cl::CommandQueue queue(context, device);
		queue.enqueueWriteBuffer(buf, CL_TRUE, 0, sizeof(double) * M * N, vector);
		queue.enqueueWriteBuffer(copyBuf, CL_TRUE, 0, sizeof(double) * M * N, copy_vector);

		cl::Kernel kernel(program, "MetalPlate");
		kernel.setArg(0, buf);
		kernel.setArg(1, copyBuf);
		kernel.setArg(2, M);
		kernel.setArg(3, N);
		kernel.setArg(4, x0);
		kernel.setArg(5, y0);
		queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(M, N), cl::NullRange);
		queue.finish();
		queue.enqueueReadBuffer(buf, CL_TRUE, 0, sizeof(double) * M * N, vector);
		bool break_cond = false;
		for (int i = 1; i < M - 1; i++) {
			for (int j = 1; j < N - 1; j++) {
				if (i != (x0 + 1) && j != (y0 + 1) && abs(vector[i * N + j] - copy_vector[i * N + j]) < 0.01) break_cond = true;
			}
		}
		if (break_cond) break;
		else {
			std::cout << "--------------------------P(" << count << ")--------------------------" << '\n';
			count++;
			for (int i = 0; i < M; i++) {
				for (int j = 0; j < N; j++) {
					std::cout << vector[i * N + j] << " ";
					copy_vector[i * N + j] = vector[i * N + j];
				}
				std::cout << '\n';
			}
		}
	}
	std::cout << '\n' << "--------------------------Convergence reached:--------------------------" << '\n';
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			std::cout << vector[i * N + j] << " ";
		}
		std::cout << '\n';
	}

	delete[] vector;
	delete[] copy_vector;
}