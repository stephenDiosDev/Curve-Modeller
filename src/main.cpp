#include <array>
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <cassert> //assert

// glad beforw glfw
#include "glad/glad.h"
//
#include "GLFW/glfw3.h"

#include "common_matrices.hpp"
//#include "obj_mesh_file_io.hpp"
//#include "obj_mesh.hpp"
#include "mat4f.hpp"
#include "mat3f.hpp"
#include "shader.hpp"
#include "program.hpp"
#include "triangle.hpp"
#include "vec3f.hpp"
#include "vec2f.hpp"
#include "shader_file_io.hpp"
#include "buffer_object.hpp"
#include "vertex_array_object.hpp"
#include "vbo_tools.hpp"
//#include "texture.hpp"
//#include "image.hpp"

using namespace math;
using namespace geometry;
using namespace opengl;

// GLOBAL Variables
Mat4f g_M = Mat4f::identity();
Mat4f g_V = Mat4f::identity();
Mat4f g_P = Mat4f::identity();



GLuint g_width = 1000, g_height = 1000;

//mouse button callback vars
int pressed = 0; //0= nothing clicked, 1=left click, 2=right click
int held = 0;	//when held is 1, the button is being held

std::vector<Vec3f> controlPoints;

std::vector<Vec3f> outCurve;

int depth = 1;

double mouseX;
double mouseY;

double xClickedPos;
double yClickedPos;

double closestPointToCursor;
double closestDistanceOfPoint;

double ndcX;
double ndcY;

// function declaration
using namespace std;

static void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos)
{
	//screenspace position of x and y for cursor
	ndcX = (xpos - ((float)g_width * (3.f / 4.f))) / ((float)g_width / 4.f);
	ndcY = (ypos - ((float)g_height) / 2.f) / (-(float)g_height / 2.f);

	//get closest point to mouse within some allowable distance
	if (pressed == 1)
	{
		//update position of where cursor was clicked
		xClickedPos = ndcX;
		yClickedPos = ndcY;

		//grab closest point to cursor
		closestDistanceOfPoint = -1;
		closestPointToCursor = -1;

		if ((ndcX <= 1.f) && (ndcX >= -1.f) && (ndcY <= 1.f) && (ndcY >= -1.f))
		{
			//go through every point and check distance to cursor
			for (int i = 0; i < controlPoints.size(); i++)
			{
				//pythagorean
				double distanceOfCurrentPointToCursor = pow(pow((ndcX - controlPoints[i].x), 2) + pow((ndcY - controlPoints[i].y), 2), 0.5f);

				if (closestDistanceOfPoint == -1)
				{
					closestDistanceOfPoint = distanceOfCurrentPointToCursor;
					closestPointToCursor = i;
				}
				else if (closestDistanceOfPoint > distanceOfCurrentPointToCursor)
				{
					closestDistanceOfPoint = distanceOfCurrentPointToCursor;
					closestPointToCursor = i;
				}
			}
		}

		pressed = 0;
	}

	//point movement
	if (held == 1)
	{
		double xMovementAmt = ndcX - xClickedPos;
		double yMovementAmt = ndcY - yClickedPos;

		controlPoints[closestPointToCursor].x = ndcX;
		controlPoints[closestPointToCursor].y = ndcY;
	}
}

/*
 * Left click (1) is selected/drag if button is held
 *
 * Right click (2) on point is point removal
 */
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		pressed = 1;
		held = 1;
	}
	//remove points
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		pressed = 2;

		xClickedPos = ndcX;
		yClickedPos = ndcY;

		//if cursor within bounds of viewport
		if ((ndcX <= 1.f) && (ndcX >= -1.f) && (ndcY <= 1.f) && (ndcY >= -1.f))
		{
			//reset distance and points
			closestDistanceOfPoint = -1;
			closestPointToCursor = -1;
			//go through every point and check distance to cursor
			for (int i = 0; i < controlPoints.size(); i++)
			{
				//pythagorean
				double distanceOfCurrentPointToCursor = pow(pow((ndcX - controlPoints[i].x), 2) + pow((ndcY - controlPoints[i].y), 2), 0.5f);

				//find closest point
				if (closestDistanceOfPoint == -1 || closestDistanceOfPoint > distanceOfCurrentPointToCursor)
				{
					closestDistanceOfPoint = distanceOfCurrentPointToCursor;
					closestPointToCursor = i;
				}
			}
		}

		//keep at only 4 control points
		if (controlPoints.size() > 4)
		{
			controlPoints.erase(controlPoints.begin() + closestPointToCursor);
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		pressed = 0;
		held = 0;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		pressed = 0;
	}
}

void setFrameBufferSize(GLFWwindow *window, int width, int height)
{
	g_width = width;
	g_height = height;
	glViewport(0, 0, g_width, g_height);
	float aspect_ratio = float(g_width) / g_height;
	g_P = orthographicProjection(-aspect_ratio, aspect_ratio, 1, -1, 0.001f, 10);
}

void setKeyboard(GLFWwindow *window, int key, int scancode, int action,
				 int mods)
{

	if (GLFW_KEY_LEFT == key)
	{
		if (GLFW_REPEAT == action || GLFW_PRESS == action)
		{
			g_M = rotateAboutYMatrix(5) * g_M;
		}
	}
	else if (GLFW_KEY_RIGHT == key)
	{
		if (GLFW_REPEAT == action || GLFW_PRESS == action)
		{
			g_M = rotateAboutYMatrix(-5) * g_M;
		}
	}
	else if (GLFW_KEY_UP == key)
	{
		if (GLFW_REPEAT == action || GLFW_PRESS == action)
		{
			g_M = uniformScaleMatrix(1.1) * g_M;
		}
	}
	else if (GLFW_KEY_DOWN == key)
	{
		if (GLFW_REPEAT == action || GLFW_PRESS == action)
		{
			g_M = uniformScaleMatrix(1. / 1.1) * g_M;
		}
	}
	else if (GLFW_KEY_W == key)
	{
		if (GLFW_REPEAT == action || GLFW_PRESS == action)
		{
			g_M = translateMatrix(0, 0.1, 0) * g_M;
		}
	}
	else if (GLFW_KEY_S == key)
	{
		if (GLFW_REPEAT == action || GLFW_PRESS == action)
		{
			g_M = translateMatrix(0, -0.1, 0) * g_M;
		}
	}
	else if (GLFW_KEY_D == key)
	{
		if (GLFW_REPEAT == action || GLFW_PRESS == action)
		{
			g_M = translateMatrix(0.1, 0, 0) * g_M;
		}
	}
	else if (GLFW_KEY_A == key)
	{
		if (GLFW_REPEAT == action || GLFW_PRESS == action)
		{
			g_M = translateMatrix(-0.1, 0, 0) * g_M;
		}
	}
	else if (GLFW_KEY_P == key)
	{
		//add point
		if (GLFW_PRESS == action)
		{
			Vec3f temp(0, 0, 0);
			controlPoints.push_back(temp);
		}
	}
	else if (GLFW_KEY_9 == key)
	{
		if (GLFW_PRESS == action)
		{
			if (depth > 1)
			{
				depth--;
			}
		}
	}
	else if (GLFW_KEY_0 == key)
	{
		if (GLFW_PRESS == action)
		{
			if (depth < 10)
			{
				depth++;
			}
		}
	}
	else if (GLFW_KEY_ESCAPE == key)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

// user defined alias
opengl::Program createShaderProgram(std::string const &vertexShaderFile,
									std::string const &fragmentShaderFile)
{
	using namespace opengl;
	auto vertexShaderSource = loadShaderStringFromFile(vertexShaderFile);
	auto fragmentShaderSource = loadShaderStringFromFile(fragmentShaderFile);

	std::cout << "[Log] compiling program " << vertexShaderFile << ' '
			  << fragmentShaderFile << '\n';
	return opengl::makeProgram(vertexShaderSource, fragmentShaderSource);
}

std::string glfwVersion()
{
	std::ostringstream s("GLFW version: ", std::ios::in | std::ios::ate);
	// print version
	int glfwMajor, glfwMinor, glfwRevision;
	glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
	s << glfwMajor << '.' << glfwMinor << '.' << glfwRevision;
	return s.str();
}

bool loadGeometryToGPU(std::vector<Vec3f> const &vertices, GLuint vboID)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(
		GL_ARRAY_BUFFER,				 // destination
		sizeof(Vec3f) * vertices.size(), // size (bytes) of memcopy to GPU
		vertices.data(),				 // pointer to data (contiguous)
		GL_STATIC_DRAW					 // usage patter of the GPU buffer
	);

	return true;
}

//takes in a vector of vec3f and rotates every vec3f in it by 5 degrees around the y axis
std::vector<Vec3f> rotateLineAroundAxis(std::vector<Vec3f> const &points, int degrees)
{
	Vec3f yAxis(0, 1, 0);

	std::vector<Vec3f> rotated;

	for (int i = 0; i < points.size(); i++)
	{
		Vec3f temp = math::rotateAroundAxis(points[i], yAxis, degrees);
		rotated.push_back(temp);
	}

	return rotated;
}

std::vector<Vec3f> createTriangleMesh(std::vector<Vec3f> const &curve)
{
	std::vector<Vec3f> points;
	std::vector<Vec3f> meshPoints;

	//we rotate by 5 degrees, so we need 72 rotations for a 360 degree object
	for (int i = 0; i < 72; i++)
	{
		std::vector<Vec3f> temp = rotateLineAroundAxis(curve, i * 5);

		//copy over result to points
		for (int j = 0; j < curve.size(); j++)
		{
			points.push_back(temp[j]);
		}
	}

	//actually create the triangle mesh now
	//loop over set of points in points

	//loop over the 72 sets of curves
    for (int i = 0; i < 72; i++)
	{

		for (int j = 0; j < curve.size() - 1; j++)
		{

            if(i == 71) {
                //first triangle
                meshPoints.push_back(points[(i * curve.size()) + j]);
                meshPoints.push_back(points[(i * curve.size()) + j + 1]);
                meshPoints.push_back(points[j]);

                //second triangle
                meshPoints.push_back(points[j]);
                meshPoints.push_back(points[(i * curve.size()) + j + 1]);
                meshPoints.push_back(points[j + 1]);
            }
            else {
                //first triangle
                meshPoints.push_back(points[(i * curve.size()) + j]);
                meshPoints.push_back(points[(i * curve.size()) + j + 1]);
                meshPoints.push_back(points[((i + 1) * curve.size()) + j]);

                //second triangle
                meshPoints.push_back(points[((i + 1) * curve.size()) + j]);
                meshPoints.push_back(points[(i * curve.size()) + j + 1]);
                meshPoints.push_back(points[((i + 1) * curve.size()) + j + 1]);
            }

		}
	}

	return meshPoints;
}

std::vector<IndicesTriangle> createIndices(std::vector<Vec3f> const &triangles) {
    std::vector<IndicesTriangle> indicesTrianglesList;

    //create indices based on triangles
    for(int i = 0; i < triangles.size() - 2; i += 3) {
        geometry::Indices pA;
        pA.vertexID() = i;
        pA.textureCoordID() = 0;
        pA.normalID() = i;

        geometry::Indices pB;
        pB.vertexID() = i + 1;
        pB.textureCoordID() = 0;
        pB.normalID() = i + 1;

        geometry::Indices pC;
        pC.vertexID() = i + 2;
        pC.textureCoordID() = 0;
        pC.normalID() = i + 2;

        geometry::Triangle_<geometry::Indices> tri({pA, pB, pC});
        indicesTrianglesList.push_back(tri);
    }

    return indicesTrianglesList;
}

std::vector<Vec3f> subdivideClosedCurve(std::vector<Vec3f> const &points, int depth)
{
	std::vector<Vec3f> out;

	return out;
}

std::vector<Vec3f> subdivideOpenCurve(std::vector<Vec3f> const &points)
{
	//guarenteed to always have minimum 4 points in points
	std::vector<Vec3f> out;

	for (int d = 0; d < depth; d++)
	{
		std::vector<Vec3f> temp;

		if (d == 0)
		{
			for (int b = 0; b < points.size(); b++)
			{
				temp.push_back(points[b]);
			}
		}
		else
		{
			for (int b = 0; b < out.size(); b++)
			{
				temp.push_back(out[b]);
			}
		}

		out.clear();
		//loop over all points, subdivide
		int i = 0;
		int j = 1;

		//chaikin
		for (i = 0, j = 1; (i < temp.size() - 1) && (j < temp.size()); i++, j++)
		{
			Vec3f pA = temp[i];
			Vec3f pB = temp[j];

			out.push_back(lerp(pA, pB, 0.25));
			out.push_back(lerp(pA, pB, 0.75));
		}
	}
	return out;
}

void setupVAO(GLuint vaoID, GLuint vboID)
{

	glBindVertexArray(vaoID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);

	// set up position input into vertex shader
	glEnableVertexAttribArray(0); // match layout # in shader
	glVertexAttribPointer(		  //
		0,						  // attribute layout # (in shader)
		3,						  // number of coordinates per vertex
		GL_FLOAT,				  // type
		GL_FALSE,				  // normalized?
		sizeof(Vec3f),			  // stride
		0						  // array buffer offset
	);

	glBindVertexArray(0);
}

GLFWwindow *initWindow()
{
	GLFWwindow *window = nullptr;

	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	std::cout << glfwVersion() << '\n';

	// set opengl version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(1000,			  // width
							  1000,			  // height
							  "Mesh Loading", // title
							  NULL,			  // unused
							  NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSwapInterval(1); // vsync
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);

	//Polygon fill mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// setup callbacks
	glfwSetFramebufferSizeCallback(window, setFrameBufferSize);
	glfwSetKeyCallback(window, setKeyboard);
	glfwSetCursorPosCallback(window, cursorPositionCallback);  //mouse pointer position
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); //keep mouse pointer visible
	glfwSetMouseButtonCallback(window, mouseButtonCallback);   //mouse button control

	return window;
}

int main()
{
	GLFWwindow *window = initWindow();

	auto vao_control = makeVertexArrayObject();
	auto vbo_control = makeBufferObject();

	auto vao_curve = makeVertexArrayObject();
	auto vbo_curve = makeBufferObject();
    auto vbo_vertices = makeBufferObject();

    GLuint totalIndices = 0;

	Vec3f viewPosition(0, 0, 3);
	g_V = lookAtMatrix(viewPosition,	// eye position
					   {0.f, 0.f, 0.f}, // look at
					   {0.f, 1.f, 0.f}  // up vector
	);
	g_P = orthographicProjection(-1, 1, 1, -1, 0.001f, 10);

	auto basicShader = createShaderProgram("../shaders/basic_vs.glsl",
										   "../shaders/basic_fs.glsl");

	auto phongShader = createShaderProgram("../shaders/phong_vs.glsl",
										   "../shaders/phong_fs.glsl");

	assert(phongShader);
    phongShader.use();

    setUniformVec3f(phongShader.uniformLocation("lightPosition"), viewPosition);
    setUniformVec3f(phongShader.uniformLocation("viewPosition"), viewPosition);


	setupVAO(vao_control.id(), vbo_control.id());
    setupVAO(vao_curve.id(), vbo_curve.id());

	controlPoints.push_back({-0.5, 0, 0});
	controlPoints.push_back({0, -0.5, 0});
	controlPoints.push_back({0.5, 0, 0});
	controlPoints.push_back({0, 0.5, 0});

	Vec3f color_curve(0, 1, 1);
	Vec3f color_control(1, 0, 0);

	//Set to one shader program
    opengl::Program *program = &phongShader;

	glPointSize(10);
	while (!glfwWindowShouldClose(window))
	{
		outCurve = subdivideOpenCurve(controlPoints);

		loadGeometryToGPU(controlPoints, vbo_control.id());

		std::vector<Vec3f> triangleMesh = createTriangleMesh(outCurve);

        geometry::OBJMesh meshData;

        meshData.triangles = createIndices(triangleMesh);
        meshData.vertices = triangleMesh;

        //cout<<"Size of triangles: "<<meshData.triangles.size()<<" Size of vertices: "<<meshData.vertices.size()<<endl;

        auto normals = geometry::calculateVertexNormals(meshData.triangles, meshData.vertices);


        auto vboData = opengl::makeConsistentVertexNormalIndices(meshData, normals);

        totalIndices = opengl::setup_vao_and_buffers(vao_curve, vbo_curve, vbo_vertices, vboData);

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		program->use();


		setUniformMat4f(program->uniformLocation("model"), g_M, true);
		setUniformMat4f(program->uniformLocation("view"), g_V, true);
		setUniformMat4f(program->uniformLocation("projection"), g_P, true);

		glViewport(0, 0, g_width / 2, g_height);

		//Curve



		vao_curve.bind();
        glDrawElements(GL_TRIANGLES,
                           totalIndices,    // # of triangles * 3
                           GL_UNSIGNED_INT, // type of indices
                           (void *)0        // offset
                           );

		glViewport(g_width / 2, 0, g_width / 2, g_height);
        //Control points
		vao_control.bind();
		glDrawArrays(GL_LINE_STRIP,		  // type of drawing (rendered to back buffer)
					 0,					  // offset into buffer
					 controlPoints.size() // number of vertices in buffer
		);

		glDrawArrays(GL_POINTS,			  // type of drawing (rendered to back buffer)
					 0,					  // offset into buffer
					 controlPoints.size() // number of vertices in buffer
		);

		glfwSwapBuffers(window); // swaps back buffer to front for drawing to screen
		glfwPollEvents();		 // will process event queue and carry on
	}

	// cleaup window, and glfw before exit
	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}
