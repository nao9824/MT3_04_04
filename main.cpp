#include <Novice.h>
#include "cmath"
#include <assert.h>
#include "imgui.h"
#define _USE_MATH_DEFINES
#include<math.h>
#include <algorithm>

const char kWindowTitle[] = "LD2B_08_ワタナベ_ナオ_タイトル";

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Matrix4x4 {
	float m[4][4];
};

struct Sphere {
	Vector3 center;//中心点
	float radius;
};

struct Ball {
	Vector3 position;
	Vector3 acceleration;
	Vector3 velocity;
	float mass;
	float radius;
	uint32_t color;
};

struct Plane {
	Vector3 normal;//法線
	float distance;//距離
};

//X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian);
//Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian);
//Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian);
//3次元アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
//逆行列
Matrix4x4 Inverse(const Matrix4x4& m);
//投資投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRadio, float nearClip, float farClip);
//ビューポート行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
//積
Matrix4x4 MatrixMultiply(const Matrix4x4& m1, const Matrix4x4& m2);
//座標変換
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
//内積
float Dot(const Vector3& v1, const Vector3& v2);
//クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2);
Vector3 Add(const Vector3& v1, const Vector3& v2);
Vector3 Subtract(const Vector3& v1, const Vector3& v2);
Vector3 Multiply(float scalar, const Vector3 v);
Vector3 vectorMultiply(const Vector3& v1, const Vector3& v2);
float Length(const Vector3& v);
Vector3 Normalize(const Vector3& v);
//正射影ベクトル
Vector3 Project(const Vector3& v1, const Vector3& v2);

Vector3 Reflect(const Vector3& input, const Vector3& nolmal);

void DrawGrit(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);
//平面の描画
Vector3 Perpendicular(const Vector3& vector);
void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

//球と平面の当たり判定
bool IsCollision(const Sphere& sphere, const Plane& plane);


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	Vector3 cameraTranslate{ 0.0f,1.9f,-6.49f };
	Vector3 cameraRotate{ 0.26f,0.0f,0.0f };
	Vector3 cameraPosition = { 0.0f,0.0f,-300.0f };
	int kWindowWidth = 1280;
	int kWindowHeight = 720;

	Sphere sphere{
		{0.0f,0.0f,0.0f},
		0.05f
	};

	Ball ball{};
	ball.position = { 0.8f,1.2f,0.3f };
	ball.acceleration = { 0.0f,-9.8f,0.0f };
	ball.mass = 2.0f;
	ball.radius = 0.05f;
	ball.color = WHITE;

	Plane plane;
	plane.normal = Normalize({ -0.2f,0.9f,-0.3f });
	plane.distance = 0.0f;

	float e = 0.8f;

	float deltaTime = 1.0f / 60.0f;
	bool startButton = false;

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = MatrixMultiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);


		if (startButton) {
			
			ball.velocity.x += ball.acceleration.x * deltaTime;
			ball.velocity.y += ball.acceleration.y * deltaTime;
			ball.velocity.z += ball.acceleration.z * deltaTime;

			ball.position.x += ball.velocity.x * deltaTime;
			ball.position.y += ball.velocity.y * deltaTime;
			ball.position.z += ball.velocity.z * deltaTime;

			if (IsCollision(Sphere{ ball.position,ball.radius }, plane)) {
				Vector3 reflected = Reflect(ball.velocity, plane.normal);
				Vector3 projectToNormal = Project(reflected, plane.normal);
				Vector3 movingDirection = Subtract(reflected, projectToNormal);
				ball.velocity = Add(Multiply(e, projectToNormal), movingDirection);
			}

		}

		Vector3 NdcVertex = Transform(sphere.center, worldViewProjectionMatrix);
		Vector3 ScreenVertice = Transform(NdcVertex, viewportMatrix);

		

		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		if (ImGui::Button("Start")) {
			startButton = true;
		}
		ImGui::End();

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrit(worldViewProjectionMatrix, viewportMatrix);

		DrawPlane(plane, worldViewProjectionMatrix, viewportMatrix, WHITE);

		DrawSphere(Sphere{ ball.position,ball.radius }, worldViewProjectionMatrix, viewportMatrix, BLUE);
		
		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}


//積
Matrix4x4 MatrixMultiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;

	result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
	result.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
	result.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
	result.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];
	result.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
	result.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
	result.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
	result.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];
	result.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
	result.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
	result.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
	result.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];
	result.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
	result.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
	result.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
	result.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];

	return result;
}
//X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 result;

	result.m[0][0] = 1;
	result.m[0][1] = 0;
	result.m[0][2] = 0;
	result.m[0][3] = 0;
	result.m[1][0] = 0;
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = std::sin(radian);
	result.m[1][3] = 0;
	result.m[2][0] = 0;
	result.m[2][1] = -std::sin(radian);
	result.m[2][2] = std::cos(radian);
	result.m[2][3] = 0;
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;

	return result;
}

//Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 result;

	result.m[0][0] = std::cos(radian);
	result.m[0][1] = 0;
	result.m[0][2] = -std::sin(radian);
	result.m[0][3] = 0;
	result.m[1][0] = 0;
	result.m[1][1] = 1;
	result.m[1][2] = 0;
	result.m[1][3] = 0;
	result.m[2][0] = std::sin(radian);
	result.m[2][1] = 0;
	result.m[2][2] = std::cos(radian);
	result.m[2][3] = 0;
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;

	return result;
}

//Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result;

	result.m[0][0] = std::cos(radian);
	result.m[0][1] = std::sin(radian);
	result.m[0][2] = 0;
	result.m[0][3] = 0;
	result.m[1][0] = -std::sin(radian);
	result.m[1][1] = std::cos(radian);
	result.m[1][2] = 0;
	result.m[1][3] = 0;
	result.m[2][0] = 0;
	result.m[2][1] = 0;
	result.m[2][2] = 1;
	result.m[2][3] = 0;
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;

	return result;
}

//3次元アフィン変換行列
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 result;
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rotateXYZMatrix = MatrixMultiply(rotateXMatrix, MatrixMultiply(rotateYMatrix, rotateZMatrix));

	result.m[0][0] = scale.x * rotateXYZMatrix.m[0][0];
	result.m[0][1] = scale.x * rotateXYZMatrix.m[0][1];
	result.m[0][2] = scale.x * rotateXYZMatrix.m[0][2];
	result.m[0][3] = 0;
	result.m[1][0] = scale.y * rotateXYZMatrix.m[1][0];
	result.m[1][1] = scale.y * rotateXYZMatrix.m[1][1];
	result.m[1][2] = scale.y * rotateXYZMatrix.m[1][2];
	result.m[1][3] = 0;
	result.m[2][0] = scale.z * rotateXYZMatrix.m[2][0];
	result.m[2][1] = scale.z * rotateXYZMatrix.m[2][1];
	result.m[2][2] = scale.z * rotateXYZMatrix.m[2][2];
	result.m[2][3] = 0;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1;

	return result;
}

//逆行列
Matrix4x4 Inverse(const Matrix4x4& m) {
	Matrix4x4 result;

	float A = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]
		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]
		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]
		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

	result.m[0][0] = (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2] - (m.m[1][3] * m.m[2][2] * m.m[3][1]) - (m.m[1][2] * m.m[2][1] * m.m[3][3]) - (m.m[1][1] * m.m[2][3] * m.m[3][2])) / A;
	result.m[0][1] = (-(m.m[0][1] * m.m[2][2] * m.m[3][3]) - (m.m[0][2] * m.m[2][3] * m.m[3][1]) - (m.m[0][3] * m.m[2][1] * m.m[3][2]) + m.m[0][3] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]) / A;
	result.m[0][2] = (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2] - (m.m[0][3] * m.m[1][2] * m.m[3][1]) - (m.m[0][2] * m.m[1][1] * m.m[3][3]) - (m.m[0][1] * m.m[1][3] * m.m[3][2])) / A;
	result.m[0][3] = (-(m.m[0][1] * m.m[1][2] * m.m[2][3]) - (m.m[0][2] * m.m[1][3] * m.m[2][1]) - (m.m[0][3] * m.m[1][1] * m.m[2][2]) + m.m[0][3] * m.m[1][2] * m.m[2][1] + m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]) / A;
	result.m[1][0] = (-(m.m[1][0] * m.m[2][2] * m.m[3][3]) - (m.m[1][2] * m.m[2][3] * m.m[3][0]) - (m.m[1][3] * m.m[2][0] * m.m[3][2]) + m.m[1][3] * m.m[2][2] * m.m[3][0] + m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]) / A;
	result.m[1][1] = (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2] - (m.m[0][3] * m.m[2][2] * m.m[3][0]) - (m.m[0][2] * m.m[2][0] * m.m[3][3]) - (m.m[0][0] * m.m[2][3] * m.m[3][2])) / A;
	result.m[1][2] = (-(m.m[0][0] * m.m[1][2] * m.m[3][3]) - (m.m[0][2] * m.m[1][3] * m.m[3][0]) - (m.m[0][3] * m.m[1][0] * m.m[3][2]) + m.m[0][3] * m.m[1][2] * m.m[3][0] + m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]) / A;
	result.m[1][3] = (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2] - (m.m[0][3] * m.m[1][2] * m.m[2][0]) - (m.m[0][2] * m.m[1][0] * m.m[2][3]) - (m.m[0][0] * m.m[1][3] * m.m[2][2])) / A;
	result.m[2][0] = (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1] - (m.m[1][3] * m.m[2][1] * m.m[3][0]) - (m.m[1][1] * m.m[2][0] * m.m[3][3]) - (m.m[1][0] * m.m[2][3] * m.m[3][1])) / A;
	result.m[2][1] = (-(m.m[0][0] * m.m[2][1] * m.m[3][3]) - (m.m[0][1] * m.m[2][3] * m.m[3][0]) - (m.m[0][3] * m.m[2][0] * m.m[3][1]) + m.m[0][3] * m.m[2][1] * m.m[3][0] + m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]) / A;
	result.m[2][2] = (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1] - (m.m[0][3] * m.m[1][1] * m.m[3][0]) - (m.m[0][1] * m.m[1][0] * m.m[3][3]) - (m.m[0][0] * m.m[1][3] * m.m[3][1])) / A;
	result.m[2][3] = (-(m.m[0][0] * m.m[1][1] * m.m[2][3]) - (m.m[0][1] * m.m[1][3] * m.m[2][0]) - (m.m[0][3] * m.m[1][0] * m.m[2][1]) + m.m[0][3] * m.m[1][1] * m.m[2][0] + m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]) / A;
	result.m[3][0] = (-(m.m[1][0] * m.m[2][1] * m.m[3][2]) - (m.m[1][1] * m.m[2][2] * m.m[3][0]) - (m.m[1][2] * m.m[2][0] * m.m[3][1]) + m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]) / A;
	result.m[3][1] = (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1] - (m.m[0][2] * m.m[2][1] * m.m[3][0]) - (m.m[0][1] * m.m[2][0] * m.m[3][2]) - (m.m[0][0] * m.m[2][2] * m.m[3][1])) / A;
	result.m[3][2] = (-(m.m[0][0] * m.m[1][1] * m.m[3][2]) - (m.m[0][1] * m.m[1][2] * m.m[3][0]) - (m.m[0][2] * m.m[1][0] * m.m[3][1]) + m.m[0][2] * m.m[1][1] * m.m[3][0] + m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]) / A;
	result.m[3][3] = (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1] - (m.m[0][2] * m.m[1][1] * m.m[2][0]) - (m.m[0][1] * m.m[1][0] * m.m[2][2]) - (m.m[0][0] * m.m[1][2] * m.m[2][1])) / A;

	return result;
}

//投資投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRadio, float nearClip, float farClip) {
	Matrix4x4 result;

	result.m[0][0] = 1 / aspectRadio * (1 / std::tan(fovY / 2));
	result.m[0][1] = 0;
	result.m[0][2] = 0;
	result.m[0][3] = 0;
	result.m[1][0] = 0;
	result.m[1][1] = 1 / std::tan(fovY / 2);
	result.m[1][2] = 0;
	result.m[1][3] = 0;
	result.m[2][0] = 0;
	result.m[2][1] = 0;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1;
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = -(nearClip * farClip) / (farClip - nearClip);
	result.m[3][3] = 0;

	return result;
}

//ビューポート行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result;

	result.m[0][0] = width / 2;
	result.m[0][1] = 0;
	result.m[0][2] = 0;
	result.m[0][3] = 0;
	result.m[1][0] = 0;
	result.m[1][1] = -(height / 2);
	result.m[1][2] = 0;
	result.m[1][3] = 0;
	result.m[2][0] = 0;
	result.m[2][1] = 0;
	result.m[2][2] = maxDepth - minDepth;
	result.m[2][3] = 0;
	result.m[3][0] = left + (width / 2);
	result.m[3][1] = top + (height / 2);
	result.m[3][2] = minDepth;
	result.m[3][3] = 1;

	return result;
}

//座標変換
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}

//グリッド線の描画
void DrawGrit(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;//Gridの半分の幅
	const uint32_t kSubdivision = 10;//分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);//一つ分の長さ
	Vector3 zLineStart;
	Vector3 zLineEnd;
	Vector3 xLineStart;
	Vector3 xLineEnd;

	//奥から手前への線を順々に引いていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		//ワールド座標系上の始点と終点を求める
		zLineStart = Vector3(xIndex * kGridEvery - kGridHalfWidth, 0, kGridHalfWidth);
		zLineEnd = Vector3(xIndex * kGridEvery - kGridHalfWidth, 0, -kGridHalfWidth);
		//スクリーン座標系まで変換をかける
		Matrix4x4 zStartWorldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, Vector3{}, zLineStart);
		Matrix4x4 zStartworldViewProjectionMatrix = MatrixMultiply(zStartWorldMatrix, viewProjectionMatrix);
		Vector3 zStartNdcVertex = Transform(Vector3{}, zStartworldViewProjectionMatrix);
		Vector3 zStartScreenVertice = Transform(zStartNdcVertex, viewportMatrix);

		Matrix4x4 zEndWorldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, Vector3{}, zLineEnd);
		Matrix4x4 zEndworldViewProjectionMatrix = MatrixMultiply(zEndWorldMatrix, viewProjectionMatrix);
		Vector3 zEndNdcVertex = Transform(Vector3{}, zEndworldViewProjectionMatrix);
		Vector3 zEndScreenVertice = Transform(zEndNdcVertex, viewportMatrix);

		if (xIndex == 5) {
			Novice::DrawLine((int)zStartScreenVertice.x, (int)zStartScreenVertice.y, (int)zEndScreenVertice.x, (int)zEndScreenVertice.y, BLACK);
		}
		else {
			Novice::DrawLine((int)zStartScreenVertice.x, (int)zStartScreenVertice.y, (int)zEndScreenVertice.x, (int)zEndScreenVertice.y, 0xAAAAAAFF);
		}
	}

	//左から右も同じように順々に引いていく
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		//ワールド座標系上の始点と終点を求める
		xLineStart = Vector3(kGridHalfWidth, 0, zIndex * kGridEvery - kGridHalfWidth);
		xLineEnd = Vector3(-kGridHalfWidth, 0, zIndex * kGridEvery - kGridHalfWidth);
		//スクリーン座標系まで変換をかける
		Matrix4x4 xStartWorldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, Vector3{}, xLineStart);
		Matrix4x4 xStartworldViewProjectionMatrix = MatrixMultiply(xStartWorldMatrix, viewProjectionMatrix);
		Vector3 xStartNdcVertex = Transform(Vector3{}, xStartworldViewProjectionMatrix);
		Vector3 xStartScreenVertice = Transform(xStartNdcVertex, viewportMatrix);

		Matrix4x4 xEndWorldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, Vector3{}, xLineEnd);
		Matrix4x4 xEndworldViewProjectionMatrix = MatrixMultiply(xEndWorldMatrix, viewProjectionMatrix);
		Vector3 xEndNdcVertex = Transform(Vector3{}, xEndworldViewProjectionMatrix);
		Vector3 xEndScreenVertice = Transform(xEndNdcVertex, viewportMatrix);

		if (zIndex == 5) {
			Novice::DrawLine((int)xStartScreenVertice.x, (int)xStartScreenVertice.y, (int)xEndScreenVertice.x, (int)xEndScreenVertice.y, BLACK);
		}
		else {
			Novice::DrawLine((int)xStartScreenVertice.x, (int)xStartScreenVertice.y, (int)xEndScreenVertice.x, (int)xEndScreenVertice.y, 0xAAAAAAFF);
		}
	}
}

void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	float pi = (float)M_PI;
	const uint32_t kSubdivision = 10; //分割数
	const float kLonEvery = pi / kSubdivision; //経度分割一つ分の角度
	const float kLatEvery = (2 * pi) / kSubdivision; //緯度分割一つ分の角度
	//ワールド座標系でのa,b,cを求める
	Vector3 a, b, c;

	//緯度の方向に分割 -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;//現在の緯度
		//経度の方向に分割 0~2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;//現在の経度
			//ワールド座標系での頂点を求める
			a = { (sphere.radius) * (cos(lat) * cos(lon)) + sphere.center.x,
				(sphere.radius) * (sin(lat)) + sphere.center.y,
				(sphere.radius) * (cos(lat) * sin(lon)) + sphere.center.z };
			b = { (sphere.radius) * (cos(lat + kLatEvery) * cos(lon)) + sphere.center.x,
				(sphere.radius) * (sin(lat + kLatEvery)) + sphere.center.y,
				(sphere.radius) * (cos(lat + kLatEvery) * sin(lon)) + sphere.center.z };
			c = { (sphere.radius) * (cos(lat) * cos(lon + kLonEvery)) + sphere.center.x,
				(sphere.radius) * (sin(lat)) + sphere.center.y,
				(sphere.radius) * (cos(lat) * sin(lon + kLonEvery)) + sphere.center.z };
			//a,b,cをスクリーン座標系まで変換
			Matrix4x4 aWorldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, Vector3{}, a);
			Matrix4x4 aworldViewProjectionMatrix = MatrixMultiply(aWorldMatrix, viewProjectionMatrix);
			Vector3 aNdcVertex = Transform(Vector3{}, aworldViewProjectionMatrix);
			Vector3 aScreenVertice = Transform(aNdcVertex, viewportMatrix);

			Matrix4x4 bWorldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, Vector3{}, b);
			Matrix4x4 bworldViewProjectionMatrix = MatrixMultiply(bWorldMatrix, viewProjectionMatrix);
			Vector3 bNdcVertex = Transform(Vector3{}, bworldViewProjectionMatrix);
			Vector3 bScreenVertice = Transform(bNdcVertex, viewportMatrix);

			Matrix4x4 cWorldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, Vector3{}, c);
			Matrix4x4 cworldViewProjectionMatrix = MatrixMultiply(cWorldMatrix, viewProjectionMatrix);
			Vector3 cNdcVertex = Transform(Vector3{}, cworldViewProjectionMatrix);
			Vector3 cScreenVertice = Transform(cNdcVertex, viewportMatrix);

			Novice::DrawLine((int)aScreenVertice.x, (int)aScreenVertice.y, (int)bScreenVertice.x, (int)bScreenVertice.y, color);
			Novice::DrawLine((int)aScreenVertice.x, (int)aScreenVertice.y, (int)cScreenVertice.x, (int)cScreenVertice.y, color);

		}
	}
}


//内積
float Dot(const Vector3& v1, const Vector3& v2) {
	Vector3 v3;
	float result;

	v3.x = v1.x * v2.x;
	v3.y = v1.y * v2.y;
	v3.z = v1.z * v2.z;

	result = v3.x + v3.y + v3.z;

	return result;
}
//クロス積
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result;

	result = { v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x };

	return result;
}
//加算
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result;

	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;

	return result;
}
//減算
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result;

	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;

	return result;
}
//スカラー倍
Vector3 Multiply(float scalar, const Vector3 v) {
	Vector3 result;

	result.x = v.x * scalar;
	result.y = v.y * scalar;
	result.z = v.z * scalar;

	return result;
}
//ベクトル同士の掛け算
Vector3 vectorMultiply(const Vector3& v1, const Vector3& v2) {
	Vector3 result;

	result.x = v1.x * v2.x;
	result.y = v1.y * v2.y;
	result.z = v1.z * v2.z;

	return result;
}
//長さ（ノルム）
float Length(const Vector3& v) {
	float result;

	result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

	return result;
}
//正規化
Vector3 Normalize(const Vector3& v) {
	float length;
	Vector3 result;

	length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	result.x = v.x / length;
	result.y = v.y / length;
	result.z = v.z / length;

	return result;
}

//正射影ベクトル
Vector3 Project(const Vector3& v1, const Vector3& v2) {
	float c{};
	Vector3 vNormalize{};
	Vector3 result{};

	vNormalize = Normalize(v2);
	c = Dot(v1, vNormalize);
	result = Multiply(c, vNormalize);

	return result;
}

Vector3 Reflect(const Vector3& input, const Vector3& nolmal) {
	Vector3 r;//反射ベクトル
	float in;

	in = 2*Dot(input, nolmal);
		
	r = Subtract(input, Multiply(in, nolmal));

	return r;

}

//平面の描画
Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return{ -vector.y,vector.x,0.0f };
	}
	return{ 0.0f,-vector.z,vector.y };
}
void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 center = Multiply(plane.distance, plane.normal);
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));
	perpendiculars[1] = { -perpendiculars[0].x,-perpendiculars[0].y,-perpendiculars[0].z };
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);
	perpendiculars[3] = { -perpendiculars[2].x,-perpendiculars[2].y,-perpendiculars[2].z };

	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = Multiply(2.0f, perpendiculars[index]);
		Vector3 point = Add(center, extend);
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
	}
	Novice::DrawLine((int)points[0].x, (int)points[0].y, (int)points[2].x, (int)points[2].y, color);
	Novice::DrawLine((int)points[2].x, (int)points[2].y, (int)points[1].x, (int)points[1].y, color);
	Novice::DrawLine((int)points[1].x, (int)points[1].y, (int)points[3].x, (int)points[3].y, color);
	Novice::DrawLine((int)points[3].x, (int)points[3].y, (int)points[0].x, (int)points[0].y, color);
}

///球と平面の当たり判定
bool IsCollision(const Sphere& sphere, const Plane& plane) {
	float nc = Dot(plane.normal, sphere.center);
	if (sqrtf((nc - plane.distance) * (nc - plane.distance)) <= sphere.radius) {
		return true;
	}
	else {
		return false;
	}
}