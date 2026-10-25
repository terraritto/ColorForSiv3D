#pragma once
#include <Siv3D.hpp>

void PlanckianLocus()
{
	constexpr double h = 6.62607015 * 1e-34;	// プランク定数
	constexpr double c = 2.99792458 * 1e+8;		// 光速
	constexpr double k = 1.380649 * 1e-23;		// ボルツマン定数

	auto PlankLaw = [h, c, k](double lambda, double T)
		{
			double L2 = lambda * lambda;
			double L5 = L2 * L2 * lambda;
			double Term1 = 2.0 * h * c * c / L5;
			double exponent = Exp(h * c / (lambda * k * T));
			double Term2 = 1.0 / (exponent - 1.0);

			return Term1 * Term2;
		};

	auto CalculateDIlluminantChromaticity = [](double CCT)
		{
			CCT *= 1.4388 / 1.438; // D65の補正
			double rCCT = 1.0 / CCT;
			Vec2 v;

			if (CCT <= 7000)
			{
				v.x = 0.244063 + (0.09911e+3 + (2.9678e+6 - 4.6070e+9 * rCCT) * rCCT) * rCCT;
			}
			else
			{
				v.x = 0.237040 + (0.24748e+3 + (1.9018e+6 - 2.0064e+9 * rCCT) * rCCT) * rCCT;
			}

			v.y = -3.000 * v.x * v.x + 2.870 * v.x - 0.275;

			return v;
		};

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

	auto CalculateKang = [](double CCT)
		{
			double T2 = CCT * CCT;
			double T3 = CCT * T2;

			Vec2 r;

			if (CCT <= 4000.0)
			{
				r.x = -0.2661239e+9 / T3 - 0.2343589e+6 / T2 + 0.8776956e+3 / CCT + 0.179910;
			}
			else if (4000.0 < CCT && CCT <= 25000.0)
			{
				r.x = -3.0258469e+9 / T3 + 2.1070379e+6 / T2 + 0.2226347e+3 / CCT + 0.240390;
			}

			double x = r.x;
			double x2 = x * x;
			double x3 = x * x2;
			if (CCT <= 2222.0)
			{
				r.y = -1.1063814 * x3 - 1.34811020 * x2 + 2.18555832 * x - 0.20219683;
			}
			else if (2222.0 < CCT && CCT <= 4000.0)
			{
				r.y = -0.9549476 * x3 - 1.37418593 * x2 + 2.09137015 * x - 0.16748867;
			}
			else if (4000.0 < CCT && CCT <= 25000.0)
			{
				r.y =  3.0817580 * x3 - 5.87338670 * x2 + 3.75112997 * x - 0.37001483;
			}

			return r;
		};

	Scene::SetBackground(Palette::White);

	Window::Resize({ 750,850 });

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

	auto sceneSize = Scene::Size();
	Vec2 sceneMin = Vec2::One() * sceneOffset;
	Vec2 sceneMax = sceneSize - sceneMin;

	Vec2 X_BOUND = Vec2{ -0.05, 0.8 };
	Vec2 Y_BOUND = Vec2{ -0.05, 0.9 };

	const Font font{ FontMethod::SDF, 10 };

	auto DrawLines = [&]()
		{
			for (int i = 0; i < wavelengthes.size(); i++)
			{
				int index = i + 1 < wavelengthes.size() ? i + 1 : 0;
				const Vec3 valueCurrent{ color_X[i], color_Y[i], color_Z[i] };
				const Vec3 valueNext{ color_X[index], color_Y[index], color_Z[index] };

				const double currentSum = valueCurrent.dot(Vec3::One());
				const double nextSum = valueNext.dot(Vec3::One());

				const double current_R = color_X[i] / currentSum, current_G = color_Y[i] / currentSum;
				const double next_R = color_X[index] / nextSum, next_G = color_Y[index] / nextSum;

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

				Line{ start, end }.overwrite(image, Palette::Red);
				Circle{ start, 2 }.overwrite(image, Palette::Blue);
				font(Format(wavelengthes[i])).paintAt
				(image, start + Line{ start, end }.normal() * offset, Palette::Black);
			}
		};

	auto DrawInsides = [&]()
		{
			struct Edge
			{
				Vec2 m_origin;
				Vec2 m_length;
			};

			Array<Edge> edges;

			for (int i = 0; i < wavelengthes.size() - 1; i++)
			{
				bool isVisible = wavelengthes[i + 1] <= 700;

				const Vec3 valueCurrent{ color_X[i], color_Y[i], color_Z[i] };
				int index = isVisible ? i + 1 : 0;
				const Vec3 valueNext{ color_X[index], color_Y[index], color_Z[index] };

				const double currentSum = valueCurrent.dot(Vec3::One());
				const double nextSum = valueNext.dot(Vec3::One());

				const double current_X = color_X[i] / currentSum, current_Y = color_Y[i] / currentSum;
				const double next_X = color_X[index] / nextSum, next_Y = color_Y[index] / nextSum;

				// 線をつくる
				Vec2 start;
				start.x = (current_X - X_BOUND.x) / (X_BOUND.y - X_BOUND.x);
				start.y = 1.0 - (current_Y - Y_BOUND.x) / (Y_BOUND.y - Y_BOUND.x);
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);

				Vec2 end;
				end.x = (next_X - X_BOUND.x) / (X_BOUND.y - X_BOUND.x);
				end.y = 1.0 - (next_Y - Y_BOUND.x) / (Y_BOUND.y - Y_BOUND.x);
				end.x = Math::Lerp(sceneMin.x, sceneMax.x, end.x);
				end.y = Math::Lerp(sceneMin.y, sceneMax.y, end.y);

				// edgeを作成
				Edge e;
				e.m_origin = start;
				e.m_length = start.xy() - end.xy();
				edges.push_back(e);

				if (!isVisible)
				{
					break;
				}
			}

			for (int x = sceneMin.x; x <= sceneMax.x; x++)
			{
				for (int y = sceneMin.x; y < sceneMax.y; y++)
				{
					bool isInside = true;
					for (const auto& edge : edges)
					{
						double compare =
							(static_cast<double>(x) - edge.m_origin.x) * edge.m_length.y
							- (static_cast<double>(y) - edge.m_origin.y) * edge.m_length.x;

						if (Abs(edge.m_length.x) <= 1 || Abs(edge.m_length.y) <= 1)
						{
							continue;
						}

						if (compare < 0.0) {
							isInside = false; break;
						}
					}

					if (isInside)
					{
						// RGをカラーに
						double X = (x - sceneMin.x) / (sceneMax.x - sceneMin.x);
						double Y = 1.0 - (y - sceneMin.y) / (sceneMax.y - sceneMin.y);
						X = Math::Lerp(X_BOUND.x, X_BOUND.y, X);
						Y = Math::Lerp(Y_BOUND.x, Y_BOUND.y, Y);

						// Bを求める
						double Z = 1.0 - X - Y;

						// XYZからRGBに戻して色にする
						Vec3 entry = Vec3{ X,Y,Z };
						Vec3 color;
						color.x = entry.dot({ 0.418466, -0.158661, -0.082835 });
						color.y = entry.dot({ -0.091169, 0.252431, 0.015707 });
						color.z = entry.dot({ 0.000921, -0.002550, 0.178599 });
						color /= color.maxComponent();

						image[y][x] = ColorF{ color,1.0 }.applySRGBCurve();
					}
				}
			}
		};

	auto DrawPlanckian = [&]()
		{
			Array<std::tuple<Vec3, double>> planckianLocus;
			constexpr double minTemperature = 1000;
			constexpr double maxTemperature = 25000;

			// 積分してPlankの法則を適用して、XYZを求める
			for (int t = minTemperature; t <= maxTemperature; t += 100)
			{
				Vec3 value = Vec3::Zero();
				for (int i = 0; i < wavelengthes.size() - 1; i++)
				{
					const double waveLength = wavelengthes[i];
					const Vec3 valueCurrent{ color_X[i], color_Y[i], color_Z[i] };
					double L = PlankLaw(waveLength * 1e-9, t);
					value += L * valueCurrent;
				}

				planckianLocus.push_back(std::make_tuple(value, t));
			}

			// 色度図空間でプロット
			for (int i = 0; i < planckianLocus.size() - 1; i++)
			{
				int next = i + 1;
				const double temperature = std::get<1>(planckianLocus[i]);
				const Vec3 valueCurrent = std::get<0>(planckianLocus[i]);
				const Vec3 valueNext = std::get<0>(planckianLocus[next]);

				const double currentSum = valueCurrent.dot(Vec3::One());
				const double nextSum = valueNext.dot(Vec3::One());

				const double current_R = valueCurrent.x / currentSum, current_G = valueCurrent.y / currentSum;
				const double next_R = valueNext.x / nextSum, next_G = valueNext.y / nextSum;

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

				Line{ start, end }.overwrite(image, Palette::Black);

				if (Math::Fmod(temperature, 1000) < 1e-15)
				{
					const double offset = 10;
					Circle{ start, 2 }.overwrite(image, Palette::Blue);
					font(Format(temperature)).paintAt
					(image, start - Line{ start, end }.normal() * offset, Palette::Black);
				}
			}
		};

	auto DrawDIlluminant = [&]()
		{
			Array<Vec2> illuminants;
			constexpr double minTemperature = 4000;
			constexpr double maxTemperature = 25000;

			// DIlluminantを適用
			for (int t = minTemperature; t <= maxTemperature; t += 100)
			{
				illuminants.push_back(CalculateDIlluminantChromaticity(t));
			}

			// 色度図空間でプロット
			for (int i = 0; i < illuminants.size(); i++)
			{
				Vec2 start = illuminants[i];

				start.x = (start.x - X_BOUND.x) / (X_BOUND.y - X_BOUND.x);
				start.y = 1.0 - (start.y - Y_BOUND.x) / (Y_BOUND.y - Y_BOUND.x);
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);

				Circle{ start, 1.5 }.overwrite(image, Palette::Black);
			}
		};

	auto DrawKrystek = [&]()
		{
			Array<Vec2> illuminants;
			constexpr double minTemperature = 1000;
			constexpr double maxTemperature = 15000;

			// Krystekのアルゴリズムを適用
			for (int t = minTemperature; t <= maxTemperature; t += 100)
			{
				illuminants.push_back(CalculateKrystek(t));
			}

			// 色度図空間でプロット
			for (int i = 0; i < illuminants.size(); i++)
			{
				Vec2 start = illuminants[i];

				start.x = (start.x - X_BOUND.x) / (X_BOUND.y - X_BOUND.x);
				start.y = 1.0 - (start.y - Y_BOUND.x) / (Y_BOUND.y - Y_BOUND.x);
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);

				Circle{ start, 1.5 }.overwrite(image, Palette::Red);
			}
		};

	auto DrawKang = [&]()
		{
			Array<Vec2> illuminants;
			constexpr double minTemperature = 1667;
			constexpr double maxTemperature = 25000;

			// Kangのアルゴリズムを適用
			for (int t = minTemperature; t <= maxTemperature; t += 100)
			{
				illuminants.push_back(CalculateKang(t));
			}

			// 色度図空間でプロット
			for (int i = 0; i < illuminants.size(); i++)
			{
				Vec2 start = illuminants[i];

				start.x = (start.x - X_BOUND.x) / (X_BOUND.y - X_BOUND.x);
				start.y = 1.0 - (start.y - Y_BOUND.x) / (Y_BOUND.y - Y_BOUND.x);
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);

				Circle{ start, 1.5 }.overwrite(image, Palette::Red);
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

	// XYZ色度図
	DrawAxis();
	DrawLines();
	DrawInsides();

	// 黒体軌跡
	DrawPlanckian();
	//DrawDIlluminant();
	//DrawKrystek();
	DrawKang();
	
	texture.fill(image);

	while (System::Update())
	{
		texture.draw();
	}
}
