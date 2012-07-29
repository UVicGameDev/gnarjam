#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <sstream>

#define sizeofv(x) (sizeof(x)/sizeof(*x))
#define PI (3.14)

struct Player
{
	sf::Sprite spr;
	sf::Vector2f vel;
	bool dueling;
	bool attackedinduel;
	float atkdist;
	Player* enemy;
	sf::Color originalcolor;
	sf::Color colorfilter;
	float hurttimer;
	int score;
};

float dist(const sf::Vector2f& v1, const sf::Vector2f& v2)
{
	sf::Vector2f dv(v1.x - v2.x, v1.y - v2.y);
	return sqrt(dv.x * dv.x + dv.y * dv.y);
}

float dist(const sf::Sprite& s1, const sf::Sprite& s2) {
	return dist(s1.GetCenter(), s2.GetCenter());
}

float sign(float x)
{
	return x > 0 ? 1 : -1;
}

int main()
{
	sf::RenderWindow app(sf::VideoMode(640,480), "gnarjam");

	sf::Image playerImg;
	playerImg.LoadFromFile("player.png");

	Player p1, p2;
	p1.spr.SetImage(playerImg);
	p2.spr.SetImage(playerImg);

	// color code players
	p1.originalcolor = sf::Color::Green;
	p2.originalcolor = sf::Color::Blue;
	p1.colorfilter = p2.colorfilter = sf::Color::White;

	p1.enemy = &p2;
	p2.enemy = &p1;

	p1.dueling = p2.dueling = false;
	p1.attackedinduel = p2.attackedinduel = false;
	p1.score = p2.score = 0;

	p1.hurttimer = p2.hurttimer = 0;

	p2.spr.SetPosition(app.GetWidth() - p2.spr.GetSize().x, app.GetHeight() - p2.spr.GetSize().y);

	Player * const players[] = {
		&p1, &p2
	};

	const sf::Key::Code player1keys[] = {
		sf::Key::A, sf::Key::S, sf::Key::D, sf::Key::F
	};

	const sf::Key::Code player2keys[] = {
		sf::Key::H, sf::Key::J, sf::Key::K, sf::Key::L
	};

	const sf::Key::Code* playerkeys[] = {
		player1keys, player2keys
	};

	while (app.IsOpened()) {
		sf::Event e;
		while (app.GetEvent(e)) {
			if (e.Type == sf::Event::Closed) {
				app.Close();
			} else if (e.Type == sf::Event::KeyPressed) {
				sf::Key::Code k = e.Key.Code;

				// handle player input
				for (size_t playerid = 0; playerid < sizeofv(playerkeys); playerid++) {
					Player* ply = players[playerid];
					const sf::Key::Code* pkeys = playerkeys[playerid];
					if (ply->vel == sf::Vector2f()) {
						int dir = 1;
						if (ply->spr.GetPosition().x > app.GetWidth()/2) {
							dir *= -1;
						}
						const float jumpvel = 2*app.GetWidth();
						const float jumpangle = PI/16;
						// jump up
						if (pkeys[0] == k) {
							ply->vel = jumpvel * sf::Vector2f(dir * cos(jumpangle), -sin(jumpangle));
						}
						// jump sideways
						if (pkeys[1] == k) {
							ply->vel = jumpvel * sf::Vector2f(dir, 0);
						}
						// jump down
						if (pkeys[2] == k) {
							ply->vel = jumpvel * sf::Vector2f(dir * cos(jumpangle), sin(jumpangle));
						}
					} else {
						// attack
						if (pkeys[3] == k) {
							if (ply->dueling && !ply->attackedinduel) {
								ply->atkdist = dist(ply->spr, ply->enemy->spr);
								ply->attackedinduel = true;
							}
						}
					}
				}
			}
		}

		float frametime = app.GetFrameTime();

		// slow down time if distance close enough
		float playerDistance = dist(p1.spr.GetPosition(), p2.spr.GetPosition());
		if (playerDistance < 128 && (sign(p1.vel.x) == -sign(p2.vel.x)) && p1.vel != sf::Vector2f() && p2.vel != sf::Vector2f() ) {
			frametime /= 3;
			p1.dueling = p2.dueling = true;
		} else {
			if (p1.dueling && p2.dueling) {
				int victor = -1;
				if (p1.attackedinduel && !p2.attackedinduel) victor = 0;
				else if (!p1.attackedinduel && p2.attackedinduel) victor = 1;
				else if (p1.attackedinduel && p2.attackedinduel) {
					if (p1.atkdist > p2.atkdist) {
						victor = 0;
					} else {
						victor = 1;
					}
				}
				if (victor != -1) {
					players[victor]->enemy->colorfilter = sf::Color::Red;
					players[victor]->enemy->hurttimer = 0.5;
					players[victor]->score++;
				}
				p1.dueling = p2.dueling = false;
				p1.attackedinduel = p2.attackedinduel = false;
			}
		}

		for (size_t playerid = 0; playerid < sizeofv(players); playerid++) {
			Player* ply = players[playerid];
			if (ply->hurttimer > 0) {
				ply->hurttimer -= frametime;
				if (ply->hurttimer <= 0) {
					ply->colorfilter = sf::Color::White;
				}
			}
		}

		// update position from velocity
		for (size_t playerid = 0; playerid < sizeofv(players); playerid++) {
			sf::Vector2f pos = players[playerid]->spr.GetPosition();
			pos += frametime * players[playerid]->vel;
			players[playerid]->spr.SetPosition(pos);
		}

		// collide with walls
		for (size_t playerid = 0; playerid < sizeofv(players); playerid++) {
			Player* ply = players[playerid];
			if (ply->spr.GetPosition().x < 0) {
				ply->spr.SetPosition(0, ply->spr.GetPosition().y);
				ply->vel = sf::Vector2f();
			} else if (ply->spr.GetPosition().x + ply->spr.GetSize().x > app.GetWidth()) {
				ply->spr.SetPosition(app.GetWidth() - ply->spr.GetSize().x, ply->spr.GetPosition().y);
				ply->vel = sf::Vector2f();
			}
		}

		app.Clear();

		// draw all players
		for (size_t playerid = 0; playerid < sizeofv(players); playerid++) {
			Player* ply = players[playerid];
			if (ply->colorfilter != sf::Color::White) {
				ply->spr.SetColor(ply->colorfilter);
			} else {
				ply->spr.SetColor(ply->originalcolor);
			}
			app.Draw(ply->spr);
		}

		std::stringstream p1ss;
		p1ss << "Player 1: " << p1.score << " points";
		std::stringstream p2ss;
		p2ss << "Player 2: " << p2.score << " points";

		sf::String p1score(p1ss.str());
		sf::String p2score(p2ss.str());
		p2score.SetPosition(app.GetWidth() - p2score.GetRect().GetWidth(), 0);

		if (p1.hurttimer > 0) {
			p2score.SetColor(sf::Color::Yellow);
		}
		if (p2.hurttimer > 0) {
			p1score.SetColor(sf::Color::Yellow);
		}

		app.Draw(p1score);
		app.Draw(p2score);

		app.Display();
	}
}
