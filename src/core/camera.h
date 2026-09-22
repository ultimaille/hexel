#pragma once
#include <ultimaille/all.h>
#include "basic.h"

using namespace UM;

struct CameraInterface{
    virtual ~CameraInterface() = default;
	virtual mat4x4  projection_matrix(float width,float height)=0;
	virtual mat4x4  view_matrix()=0;
	virtual void update()=0;
};

struct OrthographicCamera: public CameraInterface{
	double zoom = 1.;
	double rotX = 0.;
	double rotY = 0.;

	mat4x4 ortho(double  left,double right,double bottom,double top,double zNear,double zFar){
		mat4x4 m;
		m[0][0] = 2. / (right - left);
		m[1][1] = 2. / (top - bottom);
		m[2][2] = - 2. / (zFar - zNear);
		m[3][0] = - (right + left) / (right - left);
		m[3][1] = - (top + bottom) / (top - bottom);
		m[3][2] = - (zFar + zNear) / (zFar - zNear);
		return m;
	}

	virtual mat4x4 projection_matrix(float width,float height) {
		mat4x4 m = mat4x4::identity();
		m[0][0] = height/width;
		m[2][2] = .5;
		FOR(d,3)m[d][d]*=zoom;
		return m;
	}

	virtual mat4x4 view_matrix(){
		mat4x4 rx= mat4x4::identity();
		mat4x4 ry= mat4x4::identity();
		{
			double s = std::sin(rotX);
			double c = std::cos(rotX);
			rx[1][1] = c;  rx[1][2] = s;
			rx[2][1] = -s; rx[2][2] = c;
		}
		{
			double s = std::sin(rotY);
			double c = std::cos(rotY);
			ry[0][0] = c;  ry[0][2] = s;
			ry[2][0] = -s; ry[2][2] = c;
		}
		return rx*ry;
	}
	virtual void update();

};






struct Camera{
	Camera(){
		impl = std::make_unique<OrthographicCamera>();
	}
	float* projection(float width,float height){
		mat4x4 m = impl->projection_matrix(width,height);
		static float res[16]; FOR(i,16) res[i] = m[i/4][i%4]; return res;
	}
	float* view(){
		mat4x4 m = impl->view_matrix();
		static float res[16]; FOR(i,16) res[i] = m[i/4][i%4]; return res;
	}

	void update(){
		impl->update();
	}

	std::unique_ptr<CameraInterface> impl;
};
