#pragma once
#include <Siv3D.hpp>

void RGBColorSystem()
{
	Scene::SetBackground(Palette::White);

	Image image(Scene::Size(), Palette::White);
	DynamicTexture texture(image);

	CSV csvData{ U"example/csv/ciexyz31.csv" };
	if (!csvData) { throw Error(U"Failed to load csv."); }

	Array<double> wavelengthes;
	Array<double> color_R;
	Array<double> color_G;
	Array<double> color_B;

	int rows = 2; // コメント二行を飛ばした位置から開始

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

		rows++;
	}

	constexpr int offset = 15;

	auto sceneSize = Scene::Size();
	Vec2 sceneMin = Vec2{ offset,offset };
	Vec2 sceneMax = sceneSize - Vec2{ offset,offset };

	Vec2 X_BOUND = Vec2{-1.8, 1.2};
	Vec2 Y_BOUND = Vec2{-0.2, 2.2};

	auto DrawLines = [&]()
		{
			for (int i = 0; i < wavelengthes.size(); i++)
			{
				int index = i + 1 < wavelengthes.size() ? i + 1 : 0;
				const Vec3 valueCurrent{ color_R[i], color_G[i], color_B[i] };
				const Vec3 valueNext{ color_R[index], color_G[index], color_B[index] };

				const double currentSum = valueCurrent.dot(Vec3::One());
				const double nextSum = valueNext.dot(Vec3::One());

				const double current_R = color_R[i] / currentSum, current_G = color_G[i] / currentSum;
				const double next_R = color_R[index] / nextSum, next_G = color_G[index] / nextSum;

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

				const Vec3 valueCurrent{ color_R[i], color_G[i], color_B[i] };
				int index = isVisible ? i + 1 : 0;
				const Vec3 valueNext{ color_R[index], color_G[index], color_B[index] };

				const double currentSum = valueCurrent.dot(Vec3::One());
				const double nextSum = valueNext.dot(Vec3::One());

				const double current_R = color_R[i] / currentSum, current_G = color_G[i] / currentSum;
				const double next_R = color_R[index] / nextSum, next_G = color_G[index] / nextSum;

				// 線をつくる
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

						if (Abs(edge.m_length.x) <= 1e-1 || Abs(edge.m_length.y) <= 1e-1)
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
						double R = (x - sceneMin.x) / (sceneMax.x - sceneMin.x);
						double G = 1.0 - (y - sceneMin.y) / (sceneMax.y - sceneMin.y);
						R = Math::Lerp(X_BOUND.x, X_BOUND.y, R);
						G = Math::Lerp(Y_BOUND.x, Y_BOUND.y, G);

						// Bを求める
						double B = 1.0 - R - G;

						double maxElement = Max({ R,G,B });

						// 負を潰しつつ正規化
						R = Max(R/maxElement, 0.0);
						G = Max(G/maxElement, 0.0);
						B = Max(B/maxElement, 0.0);

						image[y][x] = ColorF{ R,G,B,1.0 }.gamma(2.2);
					}
				}
			}
		};

	auto DrawAxis = [&]()
		{
			// Y = 0
			{
				double y = 1.0 - (0.0 - Y_BOUND.x) / (Y_BOUND.y - Y_BOUND.x);
				Vec2 start{ 0, y };
				start.x = Math::Lerp(sceneMin.x, sceneMax.x, start.x);
				start.y = Math::Lerp(sceneMin.y, sceneMax.y, start.y);
				Vec2 end{ 1, y  };
				end.x = Math::Lerp(sceneMin.x, sceneMax.x, end.x);
				end.y = Math::Lerp(sceneMin.y, sceneMax.y, end.y);

				Line{ start, end }.overwrite(image, Palette::Black);
				
			}

			// X = 0
			{
				double x = (0.0 - X_BOUND.x) / (X_BOUND.y - X_BOUND.x);
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
