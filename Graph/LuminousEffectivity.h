#pragma once
#include <Siv3D.hpp>

void LuminousEffectivity()
{
	Scene::Resize({ 1600,900 });

	CSV csvData{ U"example/csv/logCIE2008v2q_5.csv" };
	if (!csvData) { throw Error(U"Failed to load csv."); }

	Array<double> wavelengthes;
	Array<double> values;

	for (int rows = 2; rows < csvData.rows(); rows++)
	{
		// データの抽出
		wavelengthes.push_back(Parse<double>(csvData[rows][0]));
		double entry = Parse<double>(csvData[rows][1]);

		// Log値になってるので、10^xで元に戻す
		values.push_back(Pow(10, entry));
	}

	auto AdjustValue = [](Array<double>& values)
		{
			auto maxValue = *std::max_element(values.begin(), values.end());
			auto minValue = *std::min_element(values.begin(), values.end());

			// [min, max] -> [0, 1]にスケール
			for (auto& value : values)
			{
				value = (maxValue - value) / (maxValue - minValue);
			}
		};

	AdjustValue(values);

	constexpr int offset = 15;

	auto sceneSize = Scene::Size();
	Vec2 sceneMin = Vec2{ offset,offset };
	Vec2 sceneMax = sceneSize - Vec2{ offset,offset };

	auto DrawValues = [&](const Array<double>& waves, const Array<double>& values, const ColorF color)
		{
			auto minWave = 350.0;
			auto maxWave = 850.0;

			for (int i = 0; i < waves.size() - 1; i++)
			{
				Vec2 start{ (waves[i] - minWave) / (maxWave - minWave), values[i] };
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);
				Vec2 end{ (waves[i + 1] - minWave) / (maxWave - minWave), values[i + 1] };
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

			double lineStandard = 0.01;

			for (int i = minWave; i <= maxWave; i += 1)
			{
				Vec2 start{ (i - minWave) / (maxWave - minWave), 1.0 };
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);
				Vec2 end{ (i - minWave) / (maxWave - minWave), (i - 50) % 100 == 0 ? 1.0 - lineStandard * 5 : 1.0 - lineStandard };
				end.x = Math::Lerp(sceneMin.x, sceneMax.x, end.x);
				end.y = Math::Lerp(sceneMin.y, sceneMax.y, end.y);

				Line{ start, end }.draw(Palette::White);
			}
		};

	while (System::Update())
	{
		DrawValues(wavelengthes, values, Palette::White);
		DrawColorBar();
		DrawAxis();
	}
}
