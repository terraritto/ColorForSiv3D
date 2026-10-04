#pragma once
#include <Siv3D.hpp>

void XYZColorSystem()
{
	Scene::SetBackground(Palette::White);

	Window::Resize({ 900,800 });

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

	constexpr int offset = 15;

	auto sceneSize = Scene::Size();
	Vec2 sceneMin = Vec2{ offset,offset };
	Vec2 sceneMax = sceneSize - Vec2{ offset,offset };

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

						double maxElement = Max({ X,Y,Z });

						X = Max(X / maxElement, 0.0);
						Y = Max(Y / maxElement, 0.0);
						Z = Max(Z / maxElement, 0.0);

						image[y][x] = ColorF{ X,Y,Z,1.0 };
					}
				}
			}
		};

	auto DrawAxis = [&]()
		{
			// Y
			for (double v = 0.0; v < 1.0; v+=0.1)
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
			for (double v = 0.0; v < 0.9; v+=0.1)
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
	DrawLines();
	DrawInsides();
	texture.fill(image);

	while (System::Update())
	{
		texture.draw();
	}
}
