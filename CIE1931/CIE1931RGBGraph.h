#pragma once
#include <Siv3D.hpp>

void CIE1931RGBGraph()
{
	Scene::SetBackground(Palette::White);
	Window::Resize({ 1600,900 });

	CSV csvData{ U"example/csv/ciexyz31.csv" };
	if (!csvData) { throw Error(U"Failed to load csv."); }

	Array<double> wavelengthes;
	Array<double> color_R;
	Array<double> color_G;
	Array<double> color_B;

	int rows = 2; // コメント二行を飛ばした位置から開始

	Vec3 sum = Vec3::Zero();

	while (true)
	{
		const double waveLength = Parse<double>(csvData[rows][0]);
		if (800 < waveLength) { break; }

		// データの抽出
		wavelengthes.push_back(waveLength);
		Float3 entry{ Parse<double>(csvData[rows][1]),Parse<double>(csvData[rows][2]), Parse<double>(csvData[rows][3]) };

		// XYZ->RGB変換を行う
		color_R.push_back(entry.dot({ 0.418466, -0.158661, -0.082835 }));
		color_G.push_back(entry.dot({ -0.091169, 0.252431, 0.015707 }));
		color_B.push_back(entry.dot({ 0.000921, -0.002550, 0.178599 }));

		if (Abs(waveLength - 520.0) < 1e-10)
		{
			Print << U"R: " << Format(color_R.back());
			Print << U"G: " << Format(color_G.back());
			Print << U"B: " << Format(color_B.back());
		}

		sum.x += color_R.back() * 5.0;
		sum.y += color_G.back() * 5.0;
		sum.z += color_B.back() * 5.0;

		rows++;
	}

	Logger << U"SumR: " << Format(sum.x);
	Logger << U"SumG: " << Format(sum.y);
	Logger << U"SumB: " << Format(sum.z);
	
	constexpr int offset = 15;

	auto sceneSize = Scene::Size();
	Vec2 sceneMin = Vec2{ offset,offset };
	Vec2 sceneMax = sceneSize - Vec2{ offset,offset };

	auto DrawValues = [&](const Array<double>& waves, const Array<double>& values, const ColorF color, double threshold = 1.0)
		{
			auto minWave = 350.0;
			auto maxWave = 850.0;

			auto minValue = -0.2;
			auto maxValue = 1.0;

			for (int i = 0; i < waves.size() - 1; i++)
			{
				Vec2 start{ (waves[i] - minWave) / (maxWave - minWave), 1.0-(values[i] * threshold - minValue) / (maxValue - minValue) };
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);
				Vec2 end{ (waves[i + 1] - minWave) / (maxWave - minWave), 1.0 - (values[i+1] * threshold - minValue) / (maxValue - minValue) };
				end.x = Math::Lerp(sceneMin.x, sceneMax.x, end.x);
				end.y = Math::Lerp(sceneMin.y, sceneMax.y, end.y);

				Line{ start, end }.draw(color);
			}
		};

	auto DrawColorBar = [&]()
		{
			auto minWave = 350.0;
			auto maxWave = 850.0;

			auto GetThreshold = [&](int index, double current)
				{
					double minValue = wavelengthes[index - 1];
					double maxValue = wavelengthes[index];
					return Math::Saturate((current - minValue) / (maxValue - minValue));
				};

			auto GetColor = [](const Array<double>& values, int index, double threshold)
				{
					double minValue = values[index - 1];
					double maxValue = values[index];
					return 1.0 - Math::Lerp(minValue, maxValue, threshold);
				};

			auto BrutonWaveLengthColor = [](const double waveLength)
				{
					ColorF color = Palette::Black;

					if (380.0 <= waveLength && waveLength < 440.0)
					{
						color.r = -(waveLength - 440.0) / (440.0 - 380.0);
						color.b = 1.0;
					}
					else if (440.0 <= waveLength && waveLength < 490.0)
					{
						color.g = (waveLength - 440.0) / (490.0 - 440.0);
						color.b = 1.0;
					}
					else if (490.0 <= waveLength && waveLength < 510.0)
					{
						color.g = 1.0;
						color.b = -(waveLength - 510.0) / (510.0 - 490.0);
					}
					else if (510.0 <= waveLength && waveLength < 580)
					{
						color.r = (waveLength - 510.0) / (580.0 - 510.0);
						color.g = 1.0;
					}
					else if (580.0 <= waveLength && waveLength < 645.0)
					{
						color.r = 1.0;
						color.g = -(waveLength - 645.0) / (645.0 - 580.0);
					}
					else if (645.0 <= waveLength && waveLength <= 780.0)
					{
						color.r = 1.0;
					}

					// スペクトルの端ではフェードするように
					double factor = 0.0;
					if (380.0 <= waveLength && waveLength < 420.0)
					{
						// 左端
						factor = 0.3 + 0.7 * (waveLength - 380.0) / (420.0 - 380.0);
					}
					else if (420.0 <= waveLength && waveLength < 701)
					{
						// 中間
						factor = 1.0;
					}
					else if (701.0 <= waveLength && waveLength <= 780.0)
					{
						factor = 0.3 + 0.7 * (780.0 - waveLength) / (780.0 - 701.0);
					}

					return color * factor;
				};

			for (double i = minWave; i < maxWave; i += 1.0)
			{
				Vec2 start{ (i - minWave) / (maxWave - minWave), sceneMax.y };
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);

				double next = (i + 10.0 - minWave) / (maxWave - minWave);
				next = Math::Lerp(sceneMin.x, sceneMax.x, next);

				RectF{ start.x, start.y, next - start.x, offset }.draw(BrutonWaveLengthColor(i));
			}
		};

	auto DrawAxis = [&]()
		{
			auto minWave = 350.0;
			auto maxWave = 850.0;

			auto minValue = -0.2;
			auto maxValue = 1.0;

			double threshold = 1.0 - (0.0 - minValue) / (maxValue - minValue);

			double lineStandard = 0.01;

			for (int i = minWave; i <= maxWave; i += 1)
			{
				Vec2 start{ (i - minWave) / (maxWave - minWave), threshold };
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);
				Vec2 end{ (i - minWave) / (maxWave - minWave), (i - 50) % 100 == 0 ? threshold - lineStandard * 5 : threshold - lineStandard };
				end.x = Math::Lerp(sceneMin.x, sceneMax.x, end.x);
				end.y = Math::Lerp(sceneMin.y, sceneMax.y, end.y);

				Line{ start, end }.draw(Palette::Black);
			}
		};

	bool isIntensity = false;
	while (System::Update())
	{
		DrawAxis();
		DrawValues(wavelengthes, color_R, Palette::Red);
		DrawValues(wavelengthes, color_G, Palette::Green, isIntensity ? 4.5907 : 1.0);
		DrawValues(wavelengthes, color_B, Palette::Blue, isIntensity ? 0.0601 : 1.0);
		DrawColorBar();

		if (KeySpace.down())
		{
			isIntensity = !isIntensity;
		}
	}
}
