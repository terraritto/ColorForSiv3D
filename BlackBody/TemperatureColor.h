#pragma once
#include <Siv3D.hpp>

void TemperatureColor()
{
	auto CalculateKrystek = [](double CCT)
		{
			double T2 = CCT * CCT;

			// uvを計算
			double u = (0.860117757 + 1.54118254e-4 * CCT + 1.28641212e-7 * T2) / (1.0 + 8.42420235e-4 * CCT + 7.08145163e-7 * T2);
			double v = (0.317398726 + 4.22806245e-5 * CCT + 4.20481691e-8 * T2) / (1.0 - 2.89741816e-5 * CCT + 1.61456053e-7 * T2);

			// uvをxyにする
			double x = 3.0 * u / (2.0 * u - 8.0 * v + 4.0);
			double y = 2.0 * v / (2.0 * u - 8.0 * v + 4.0);

			return Vec2{ x,y };
		};

	Scene::SetBackground(Palette::White);

	Window::Resize({ 900,50 });

	Image image(Scene::Size(), Palette::White);
	DynamicTexture texture(image);

	CSV csvData{ U"example/csv/ciexyz31.csv" };
	if (!csvData) { throw Error(U"Failed to load csv."); }

	Array<double> wavelengthes;
	Array<double> color_X;
	Array<double> color_Y;
	Array<double> color_Z;

	int rows = 2; // コメント二行を飛ばした位置から開始

	while (true)
	{
		const double waveLength = Parse<double>(csvData[rows][0]);
		if (800 < waveLength) { break; }

		// データの抽出
		wavelengthes.push_back(waveLength);
		Float3 entry{ Parse<double>(csvData[rows][1]),Parse<double>(csvData[rows][2]), Parse<double>(csvData[rows][3]) };

		// XYZなので、そのまま突っ込む
		color_X.push_back(entry.x);
		color_Y.push_back(entry.y);
		color_Z.push_back(entry.z);

		rows++;
	}

	constexpr int sceneOffset = 15;

	Vec2 sceneMin = Vec2::Zero();
	Vec2 sceneMax = Scene::Size();

	Vec2 X_BOUND = Vec2{ -0.05, 0.8 };

	auto DrawKrystek = [&]()
		{
			constexpr double minTemperature = 1000;
			constexpr double maxTemperature = 15000;

			for (int t = minTemperature; t < maxTemperature; t += 100)
			{
				// Krystekのアルゴリズムを適用
				Vec2 v = CalculateKrystek(t);

				// Bを求める
				double Z = 1.0 - v.x - v.y;

				// XYZからRGBに戻して色にする
				Vec3 entry = Vec3{ v.x,v.y,Z };
				Vec3 color;
				color.x = entry.dot({ 3.2404542, -1.5371385, -0.4985314});
				color.y = entry.dot({ -0.9692660, 1.8760108, 0.0415560 });
				color.z = entry.dot({ 0.0556434, -0.2040259, 1.0572252 });
				color /= color.maxComponent();

				Vec2 start, end;
				start.x = (t - minTemperature) / (maxTemperature - minTemperature) * sceneMax.x;
				start.y = 0.0;
				end.x = (t + 100 - minTemperature) / (maxTemperature - minTemperature) * sceneMax.x;
				end.y = sceneMax.y;

				RectF{ start, end }.paint(image, ColorF{ color, 1.0 }.applySRGBCurve());
			}
		};

	DrawKrystek();
	texture.fill(image);

	while (System::Update())
	{
		texture.draw();
	}
}
