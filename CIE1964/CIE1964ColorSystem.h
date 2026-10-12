#pragma once
#include <Siv3D.hpp>

void CIE1964ColorSystem()
{
	Scene::SetBackground(Palette::White);

	Window::Resize({ 900,800 });

	Image image(Scene::Size(), Palette::White);
	DynamicTexture texture(image);

	auto LoadCIEData = [](const String& path, int rows, Array<double>* wave, Array<double>& X, Array<double>& Y, Array<double>& Z)
		{
			CSV csvData{ path };
			if (!csvData) { throw Error(U"Failed to load csv."); }

			while (true)
			{
				const double waveLength = Parse<double>(csvData[rows][0]);
				if (800 < waveLength) { break; }

				// データの抽出
				if (wave) { wave->push_back(waveLength); }
				Float3 entry{ Parse<double>(csvData[rows][1]),Parse<double>(csvData[rows][2]), Parse<double>(csvData[rows][3]) };

				// XYZなので、そのまま突っ込む
				X.push_back(entry.x);
				Y.push_back(entry.y);
				Z.push_back(entry.z);

				rows++;
			}
		};

	// 波長
	Array<double> wavelengthes;

	// CIE1931 XYZ 2Deg
	Array<double> CIE1931_X;
	Array<double> CIE1931_Y;
	Array<double> CIE1931_Z;
	// ここはrowsは2行コメントを飛ばす必要有
	LoadCIEData(U"example/csv/ciexyz31.csv", 2, nullptr, CIE1931_X, CIE1931_Y, CIE1931_Z);

	// CIE1964 XYZ 10Deg
	Array<double> CIE1964_X;
	Array<double> CIE1964_Y;
	Array<double> CIE1964_Z;
	// ここはrowsが0から読みこむ必要有
	LoadCIEData(U"example/csv/ciexyz64.csv", 0, &wavelengthes, CIE1964_X, CIE1964_Y, CIE1964_Z);

	constexpr int offset = 15;

	auto sceneSize = Scene::Size();
	Vec2 sceneMin = Vec2{ offset,offset };
	Vec2 sceneMax = sceneSize - Vec2{ offset,offset };

	Vec2 X_BOUND = Vec2{ -0.05, 0.8 };
	Vec2 Y_BOUND = Vec2{ -0.05, 0.9 };

	const Font font{ FontMethod::SDF, 10 };

	auto DrawLines = [&](
		const Array<double>& X, const Array<double>& Y, const Array<double>& Z,
		const ColorF& lineColor, const ColorF& circleColor)
		{
			for (int i = 0; i < wavelengthes.size(); i++)
			{
				int index = i + 1 < wavelengthes.size() ? i + 1 : 0;
				const Vec3 valueCurrent{ X[i], Y[i], Z[i] };
				const Vec3 valueNext{ X[index], Y[index], Z[index] };

				const double currentSum = valueCurrent.dot(Vec3::One());
				const double nextSum = valueNext.dot(Vec3::One());

				const double current_R = X[i] / currentSum, current_G = Y[i] / currentSum;
				const double next_R = X[index] / nextSum, next_G = Y[index] / nextSum;

				const double offset = 10;

				// R-G空間でプロット
				Vec2 start;
				start.x = (current_R - X_BOUND.x) / (X_BOUND.y - X_BOUND.x);
				start.y = 1.0 - (current_G - Y_BOUND.x) / (Y_BOUND.y - Y_BOUND.x);
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);

				Vec2 end;
				end.x = (next_R - X_BOUND.x) / (X_BOUND.y - X_BOUND.x);
				end.y = 1.0 - (next_G - Y_BOUND.x) / (Y_BOUND.y - Y_BOUND.x);
				end.x = Math::Lerp(sceneMin.x, sceneMax.x, end.x);
				end.y = Math::Lerp(sceneMin.y, sceneMax.y, end.y);

				Line{ start, end }.overwrite(image, lineColor);
				Circle{ start, 2 }.overwrite(image, circleColor);
				font(Format(wavelengthes[i])).paintAt
				(image, start + Line{ start, end }.normal() * offset, Palette::Black);
			}
		};

	auto DrawAxis = [&]()
		{
			// Y
			for (double v = 0.0; v < 1.0; v += 0.1)
			{
				double y = 1.0 - (v - Y_BOUND.x) / (Y_BOUND.y - Y_BOUND.x);
				Vec2 start{ 0, y };
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);
				Vec2 end{ 1, y };
				end.x = Math::Lerp(sceneMin.x, sceneMax.x, end.x);
				end.y = Math::Lerp(sceneMin.y, sceneMax.y, end.y);

				Line{ start, end }.overwrite(image, Palette::Black);

			}

			// X
			for (double v = 0.0; v < 0.9; v += 0.1)
			{
				double x = (v - X_BOUND.x) / (X_BOUND.y - X_BOUND.x);
				Vec2 start{ x, 0 };
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);
				Vec2 end{ x, 1 };
				end.x = Math::Lerp(sceneMin.x, sceneMax.x, end.x);
				end.y = Math::Lerp(sceneMin.y, sceneMax.y, end.y);

				Line{ start, end }.overwrite(image, Palette::Black);
			}
		};

	DrawAxis();
	DrawLines(CIE1931_X, CIE1931_Y, CIE1931_Z, Palette::Blue, Palette::Black);
	DrawLines(CIE1964_X, CIE1964_Y, CIE1964_Z, Palette::Red, Palette::Black);
	texture.fill(image);

	while (System::Update())
	{
		texture.draw();
	}
}
