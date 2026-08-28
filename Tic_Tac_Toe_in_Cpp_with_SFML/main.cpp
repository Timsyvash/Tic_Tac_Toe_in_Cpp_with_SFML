#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>

const char* title = "Tic Tac Toe";

using namespace std;
using namespace sf;

enum CellState {
	Empty,
	Cross,
	Nought
};

struct GameContext {
	unique_ptr<RenderWindow> m_window;

	GameContext() {
		m_window = make_unique<RenderWindow>();
	}
};

class Game {
private:
	shared_ptr<GameContext> context;

	Font font;
	Text pause_text;
	Text game_over_text;
	Text win_text;
public:
	float cell_size, padding, total_size;
	vector<RectangleShape> gridLines;

	CellState game_board[3][3] = {
		{Empty, Empty, Empty},
		{Empty, Empty, Empty},
		{Empty, Empty, Empty},
	};

	bool is_cross_turn;
	bool is_game_over;
	bool is_pause;
	bool is_win;

	SoundBuffer game_over_sound_buffer;
	SoundBuffer pause_sound_buffer;
	SoundBuffer win_sound_buffer;
	SoundBuffer mouse_click_sound_buffer;

	Sound game_over_sound;
	Sound pause_sound;
	Sound win_sound;
	Sound mouse_click_sound;

	Game(shared_ptr<GameContext> context_in) : context(context_in) {
		cell_size = 200.0f;
		padding = 8.0f;
		is_cross_turn = true;
		is_game_over = false;
		is_pause = false;
		is_win = false;

		total_size = cell_size * 3 + padding * 2;

		for (int i = 1; i < 3; ++i) {
			RectangleShape vLine(Vector2f(padding, cell_size * 3 + padding * 2));
			vLine.setPosition(i * cell_size + (i - 1) * padding, 0);
			vLine.setFillColor(Color::White);
			gridLines.push_back(vLine);

			RectangleShape hLine(Vector2f(cell_size * 3 + padding * 2, padding));
			hLine.setPosition(0, i * cell_size + (i - 1) * padding);
			hLine.setFillColor(Color::White);
			gridLines.push_back(hLine);
		}

		font.loadFromFile("Font/ZvinSerif-Regular.ttf");

		pause_text.setFont(font);
		pause_text.setString("PAUSE. Press P to resume");
		pause_text.setCharacterSize(30);
		pause_text.setFillColor(Color::White);
		pause_text.setPosition(total_size / 2 - 180, total_size / 2 - 30);

		game_over_text.setFont(font);
		game_over_text.setString("GAME OVER. Press R to restart");
		game_over_text.setCharacterSize(30);
		game_over_text.setFillColor(Color::Red);
		game_over_text.setPosition(total_size / 2 - 200, total_size / 2 - 30);

		win_text.setFont(font);
		win_text.setCharacterSize(30);
		win_text.setFillColor(Color::Green);

		// Завантаження звуків з перевіркою
		if (!game_over_sound_buffer.loadFromFile("Sounds/game_over.wav")) {
			cerr << "Failed to load game_over.wav" << endl;
		}
		else {
			game_over_sound.setBuffer(game_over_sound_buffer);
			game_over_sound.setVolume(25.0f);
		}

		if (!pause_sound_buffer.loadFromFile("Sounds/pause.wav")) {
			cerr << "Failed to load pause.wav" << endl;
		}
		else {
			pause_sound.setBuffer(pause_sound_buffer);
			pause_sound.setVolume(25.0f);
		}

		if (!mouse_click_sound_buffer.loadFromFile("Sounds/mouse_click.wav")) {
			cerr << "Failed to load mouse_click.wav" << endl;
		}
		else {
			mouse_click_sound.setBuffer(mouse_click_sound_buffer);
			mouse_click_sound.setVolume(25.0f);
		}

		if (!win_sound_buffer.loadFromFile("Sounds/win.wav")) {
			cerr << "Failed to load win.wav" << endl;
		}
		else {
			win_sound.setBuffer(win_sound_buffer);
			win_sound.setVolume(25.0f);
		}
	}

	void TogglePause() {
		if (!is_game_over && !is_win) {
			is_pause = !is_pause;
			pause_sound.play();
		}
	}

	void Reset() {
		is_cross_turn = true;
		is_game_over = false;
		is_pause = false;
		is_win = false;

		for (int row = 0; row < 3; ++row) {
			for (int col = 0; col < 3; ++col) {
				game_board[row][col] = Empty;
			}
		}
	}

	void CheckWinOrDraw() {
		CellState winner = Empty;

		// Перевірка рядків та стовпців
		for (int i = 0; i < 3; ++i) {
			if (game_board[i][0] != Empty && game_board[i][0] == game_board[i][1] && game_board[i][1] == game_board[i][2]) {
				winner = game_board[i][0];
				is_win = true;
				break;
			}
			if (game_board[0][i] != Empty && game_board[0][i] == game_board[1][i] && game_board[1][i] == game_board[2][i]) {
				winner = game_board[0][i];
				is_win = true;
				break;
			}
		}

		// Перевірка діагоналей
		if (!is_win) {
			if (game_board[0][0] != Empty && game_board[0][0] == game_board[1][1] && game_board[1][1] == game_board[2][2]) {
				winner = game_board[0][0];
				is_win = true;
			}
			else if (game_board[0][2] != Empty && game_board[0][2] == game_board[1][1] && game_board[1][1] == game_board[2][0]) {
				winner = game_board[0][2];
				is_win = true;
			}
		}

		// Якщо хтось виграв
		if (is_win) {
			if (winner == Cross) {
				win_text.setString("CROSS WINS! Press R");
			}
			else {
				win_text.setString("NOUGHT WINS! Press R");
			}
			win_text.setPosition(total_size / 2 - 160, total_size / 2 - 30);
			win_sound.play(); // Відтворюємо звук перемоги
			return;
		}

		// Перевірка на нічию
		bool is_full = true;
		for (int row = 0; row < 3; ++row) {
			for (int col = 0; col < 3; ++col) {
				if (game_board[row][col] == Empty) {
					is_full = false;
					break;
				}
			}
		}

		if (is_full) {
			is_game_over = true;
			game_over_sound.play(); // Відтворюємо звук нічиєї (Game Over)
		}
	}

	void Draw() {
		for (const auto& line : gridLines) {
			context->m_window->draw(line);
		}

		for (int row = 0; row < 3; ++row) {
			for (int col = 0; col < 3; ++col) {
				float centerX = col * (cell_size + padding) + cell_size / 2.0f + padding / 2.0f;
				float centerY = row * (cell_size + padding) + cell_size / 2.0f + padding / 2.0f;

				if (game_board[row][col] == Cross) {
					float lineLength = cell_size * 0.6f;
					RectangleShape l1(Vector2f(lineLength, 8.0f));
					RectangleShape l2(Vector2f(lineLength, 8.0f));

					l1.setOrigin(lineLength / 2.0f, 4.0f);
					l2.setOrigin(lineLength / 2.0f, 4.0f);

					l1.setPosition(centerX, centerY);
					l2.setPosition(centerX, centerY);

					l1.setRotation(45);
					l2.setRotation(-45);

					l1.setFillColor(Color::Blue);
					l2.setFillColor(Color::Blue);

					context->m_window->draw(l1);
					context->m_window->draw(l2);
				}
				else if (game_board[row][col] == Nought) {
					float radius = cell_size * 0.3f;
					CircleShape outerCircle(radius);
					outerCircle.setOrigin(radius, radius);
					outerCircle.setPosition(centerX, centerY);
					outerCircle.setFillColor(Color::Red);

					float innerRadius = radius - 8.0f;
					CircleShape innerCircle(innerRadius);
					innerCircle.setOrigin(innerRadius, innerRadius);
					innerCircle.setPosition(centerX, centerY);
					innerCircle.setFillColor(Color::Black);

					context->m_window->draw(outerCircle);
					context->m_window->draw(innerCircle);
				}
			}
		}

		if (is_pause) context->m_window->draw(pause_text);
		if (is_game_over) context->m_window->draw(game_over_text);
		if (is_win) context->m_window->draw(win_text);
	}
};

int main() {
	shared_ptr<GameContext> game_context = make_shared<GameContext>();

	Game game(game_context);

	game_context->m_window->create(VideoMode(game.cell_size * 3 + game.padding * 2,
		game.cell_size * 3 + game.padding * 2), title);

	while (game_context->m_window->isOpen())
	{
		Event event;
		while (game_context->m_window->pollEvent(event)) {
			if (event.type == Event::Closed)
				game_context->m_window->close();

			if (event.type == Event::MouseButtonPressed) {
				if (event.mouseButton.button == Mouse::Left) {
					if (game.is_pause || game.is_game_over || game.is_win) continue;
					short mouse_x = event.mouseButton.x;
					short mouse_y = event.mouseButton.y;

					short col = mouse_x / (game.cell_size + game.padding);
					short row = mouse_y / (game.cell_size + game.padding);

					if (row >= 0 && row < 3 && col >= 0 && col < 3) {
						if (game.game_board[row][col] == Empty) {
							if (game.is_cross_turn) {
								game.game_board[row][col] = Cross;
							}
							else {
								game.game_board[row][col] = Nought;
							}
							game.is_cross_turn = !game.is_cross_turn;

							game.CheckWinOrDraw();
						}
					}

					game.mouse_click_sound.setBuffer(game.mouse_click_sound_buffer);
					game.mouse_click_sound.setVolume(25.0f);
					game.mouse_click_sound.play();
				}
			}

			if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::R) {
					game.Reset();
				}
				if (event.key.code == Keyboard::P) {
					game.TogglePause();
				}
			}
		}

		game_context->m_window->clear(Color::Black);
		game.Draw();
		game_context->m_window->display();
	}

	return 0;
}