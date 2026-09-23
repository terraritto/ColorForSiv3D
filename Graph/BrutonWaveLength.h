#pragma once
#include <Siv3D.hpp>

void BrutonWaveLength()
{
	Window::Resize({ 300,15 });

	constexpr int offset = 15;

	auto sceneSize = Scene::Size();
	Vec2 sceneMin = Vec2{ offset,offset };
	Vec2 sceneMax = sceneSize - Vec2{ offset,offset };

	auto DrawColorBar = [&]()
		{
			auto minWave = 350.0;
			auto maxWave = 850.0;

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

	while (System::Update())
	{
		DrawColorBar();
	}
}
