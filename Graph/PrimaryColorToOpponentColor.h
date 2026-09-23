#pragma once
#include <Siv3D.hpp>

void PrimaryColorToOpponentColor()
{
	constexpr int offset = 15;

	Scene::Resize({ 900 + offset * 2,900 + offset * 2 });

	CSV csvData{ U"example/csv/sp.csv" };
	if (!csvData) { throw Error(U"Failed to load csv."); }

	Array<double> wavelengthes;
	Array<double> coneSpectralSensitivites_L;
	Array<double> coneSpectralSensitivites_M;
	Array<double> coneSpectralSensitivites_S;

	int rows = 2; // コメント二行を飛ばした位置から開始

	while (true)
	{
		const double waveLength = Parse<double>(csvData[rows][0]);
		if (800 < waveLength) { break; }

		// データの抽出
		wavelengthes.push_back(waveLength);
		Vec3 entry{ Parse<double>(csvData[rows][1]),Parse<double>(csvData[rows][2]), Parse<double>(csvData[rows][3]) };

		// Log値になってるので、10^xで元に戻す
		coneSpectralSensitivites_L.push_back(Pow(10, entry.x));
		coneSpectralSensitivites_M.push_back(Pow(10, entry.y));
		coneSpectralSensitivites_S.push_back(Pow(10, entry.z));
		rows++;
	}

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

	AdjustValue(coneSpectralSensitivites_L);
	AdjustValue(coneSpectralSensitivites_M);
	AdjustValue(coneSpectralSensitivites_S);

	auto sceneSize = Scene::Size();
	Vec2 sceneMin = Vec2{ offset,offset };
	Vec2 sceneMax = sceneSize - Vec2{ offset,offset };

	constexpr double MIN_VALUE = -1.0;
	constexpr double MAX_VALUE = 1.0;
	constexpr double MIN_WAVE = 350.0;
	constexpr double MAX_WAVE = 850.0;

	auto DrawAxis = [&]()
		{
			double axisOffset = 0.1;

			// Y-Axis Horizontal
			for (double x = MIN_VALUE; x <= MAX_VALUE; x += axisOffset)
			{
				Vec2 start{ (x - MIN_VALUE) / (MAX_VALUE - MIN_VALUE), sceneMin.y };
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				Vec2 end{ (x - MIN_VALUE) / (MAX_VALUE - MIN_VALUE), sceneMax.y };
				end.x = Math::Lerp(sceneMin.x, sceneMax.x, end.x);

				Line{ start, end }.draw(Palette::White);
			}

			// X-Axis Horizontal
			for (double y = MIN_VALUE; y <= MAX_VALUE; y += axisOffset)
			{
				Vec2 start{ sceneMin.x, (y - MIN_VALUE) / (MAX_VALUE - MIN_VALUE) };
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);
				Vec2 end{ sceneMax.x, (y - MIN_VALUE) / (MAX_VALUE - MIN_VALUE) };
				end.y = Math::Lerp(sceneMin.y, sceneMax.y, end.y);

				Line{ start, end }.draw(Palette::White);
			}
		};

	constexpr double CIRCLE_RADIUS = 20;
	auto DrawCircle = [&](const Vec2& value, const ColorF& color)
		{
			Vec2 pos = (value - Vec2::One() * MIN_VALUE) / (MAX_VALUE - MIN_VALUE);
			pos.x = Math::Lerp(sceneMin.x, sceneMax.x, pos.x);
			pos.y = Math::Lerp(sceneMin.y, sceneMax.y, 1.0 - pos.y);

			Circle{ pos, CIRCLE_RADIUS }.draw(color);
		};

	Timer timer{ 10s };
	timer.start();

	auto CalculateOpponentColorPosition = [&](Vec2& pos, ColorF& color)
		{
			ClearPrint();
			if (timer.reachedZero()) { timer.restart(); }

			double wave = Math::Lerp(MIN_WAVE, MAX_WAVE, timer.progress0_1());
			int index = std::distance(wavelengthes.begin(),
				std::find_if(wavelengthes.begin(), wavelengthes.end(), [wave](const double& v)
				{
					return wave < v;
				}));

			// 境界条件での調整
			if (index == 0) { index++; }
			if (wavelengthes.size() <= index) { index = wavelengthes.size() - 1; }

			// 区間上から[0,1]に変換
			double threshold = (wave - wavelengthes[index - 1]) / (wavelengthes[index] - wavelengthes[index - 1]);
			threshold = Math::Saturate(threshold);

			// Lookup
			Vec3 lms = Vec3::Zero();
			lms.x = Math::Lerp(coneSpectralSensitivites_L[index - 1], coneSpectralSensitivites_L[index], threshold);
			lms.y = Math::Lerp(coneSpectralSensitivites_M[index - 1], coneSpectralSensitivites_M[index], threshold);
			lms.z = Math::Lerp(coneSpectralSensitivites_S[index - 1], coneSpectralSensitivites_S[index], threshold);

			// AdjustValue分の調整
			lms = Vec3::One() - lms;

			// 線形化
			constexpr double X_CONSTANT = -0.7;
			constexpr double Y_CONSTANT = -1.6;
			pos.x = X_CONSTANT * (lms.z - lms.y);
			pos.y = Y_CONSTANT * (lms.y - lms.x);

			color = BrutonWaveLengthColor(wave);

			// 現所の値は描画しとく
			Print << U"WaveLength: " + Format(wave);
		};

	Vec2 pos; ColorF color;

	while (System::Update())
	{
		// Axis
		DrawAxis();

		// Standard Color
		DrawCircle({  0.0,  1.0 }, Palette::Red);
		DrawCircle({ -1.0,  0.0 }, Palette::Blue);
		DrawCircle({  0.0, -1.0 }, Palette::Green);
		DrawCircle({  1.0,  0.0 }, Palette::Yellow);

		// current
		CalculateOpponentColorPosition(pos, color);
		DrawCircle(pos, color);
	}
}
