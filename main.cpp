#include <SFML/Graphics.hpp>
#include  <olcUTIL_Geometry2D.h>
#include <vector>
#include <algorithm>
#include <memory>
using namespace olc::utils::geom2d;

sf::Texture playerTex;
sf::Texture tileTex;

enum class TexType
{
	Player,
	Tile,
	NotSet
};

struct rec
{
	olc::vf2d pos{};
	olc::vf2d size{};
	TexType texType{ TexType::NotSet };
	olc::vf2d vel{};
	rec() = default;
	~rec() = default;
	rec(const rec&) = default;
	rec(rec&&) = default;
	rec& operator=(const rec&) = default;
	rec& operator=(rec&&) = default;
	inline void set(olc::vf2d pos_, olc::vf2d size_, TexType texType_, olc::vf2d vel_ = { 0.f,0.f })
	{
		pos = pos_;
		size = size_;
		texType = texType_;
		vel = vel_;
	}
};

struct aRay
{
	olc::vf2d origin;
	olc::vf2d endPt;
	aRay(olc::vf2d start, olc::vf2d end)
	{
		origin = start;
		endPt = end;
	}
	olc::vf2d D()
	{
		return endPt - origin;
	}
	olc::vf2d orig()
	{
		return origin;
	}
	olc::vf2d dir()
	{
		return D().norm();
	}
};

std::unique_ptr<sf::Sprite> spr(const rec& r)
{
	std::unique_ptr<sf::Sprite> sprite = std::make_unique<sf::Sprite>();
	switch (r.texType)
	{
	case TexType::Player:
		sprite->setTexture(playerTex);
		break;
	case TexType::Tile:
		sprite->setTexture(tileTex);
		break;
	default:
		break;
	}
	sprite->setPosition({ r.pos.x, r.pos.y });
	sprite->setTextureRect({ {0, 0},{(int)r.size.x, (int)r.size.y} });
	return std::move(sprite);
}		


bool PointVsRect(const olc::vf2d& p, const rec& r)
{
	return (p.x >= r.pos.x && p.y >= r.pos.y && p.x < r.pos.x + r.size.x && p.y < r.pos.y + r.size.y);
}

bool RectVsRect(const rec& r1, const rec& r2)
{
	return (r1.pos.x < r2.pos.x + r2.size.x && r1.pos.x + r1.size.x > r2.pos.x && r1.pos.y < r2.pos.y + r2.size.y && r1.pos.y + r1.size.y > r2.pos.y);
}

bool RayVsRect(const olc::vf2d& ray_origin, const olc::vf2d& ray_dir, const rec& target, olc::vf2d& contact_point, olc::vi2d& contact_normal, float& t_hit_near)
{
	olc::vf2d t_near = (target.pos - ray_origin) / ray_dir;
	olc::vf2d t_far = (target.pos +target.size - ray_origin) / ray_dir;
	if (std::isnan(t_far.y) || std::isnan(t_far.x)) return false;
	if (std::isnan(t_near.y) || std::isnan(t_near.x)) return false;
	if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
	if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);
	if (t_near.x > t_far.y || t_near.y > t_far.x) return false;
	t_hit_near = std::max(t_near.x, t_near.y);
	float t_hit_far = std::min(t_far.x, t_far.y);
	if (t_hit_far < 0) return false;
	contact_point = ray_origin + t_hit_near * ray_dir;
	if (t_near.x > t_near.y)
	{
		if (ray_dir.x < 0)
		{
			contact_normal = { 1, 0 };
		}
		else
		{
			contact_normal = { -1, 0 };
		}
	}
	else if (t_near.x < t_near.y)
	{
		if (ray_dir.y < 0)
		{
			contact_normal = { 0, 1 };
		}
		else
		{
			contact_normal = { 0, -1 };
		}
	}
	return true;
}

bool DynamicRectVsRect(const rec& in, const rec& target, olc::vf2d& contact_point, olc::vi2d& contact_normal, float& contact_time, float fElapsedTime)
{
	if (in.vel.x == 0 && in.vel.y == 0)
	{
		return false;
	}
	rec expanded_target;
	expanded_target.pos = target.pos - in.size / 2.f;
	expanded_target.size = target.size + in.size;
	if (RayVsRect(in.pos + in.size / 2.f, in.vel * fElapsedTime, expanded_target, contact_point, contact_normal, contact_time))
	{
		if (contact_time <= 1.0f && contact_time >= 0.f)
			return true;
	}
	return false;
}
int main()
{
	sf::RenderWindow wnd{ {1600U,900U,32U}, "Collisions" };
	playerTex.loadFromFile("assets/player.png");
	tileTex.loadFromFile("assets/tileset.png");
	std::vector<rec> objs;
	objs.clear();
	{
		rec pr;
		pr.set({ 400.f, 400.f }, { 124.f, 180.f }, TexType::Player, { 0.f,0.f });
		objs.push_back(pr);
	}
	objs.reserve(static_cast<std::vector<rec, std::allocator<rec>>::size_type>(1) + 30);
	for (int i = 1; i < 20; i++)
	{
		objs.emplace_back(rec{});
		objs[i].set({ ((float)i * 64.f) + 200.f, 900.f - 256.f }, { 64.f,64.f }, TexType::Tile, { 0.f,0.f });
	}
	for (int i = 20; i < 25; i++)
	{
		objs.emplace_back(rec{});
		objs[i].set({ 200.f,  900.f - 256.f - (((i-20) + 1) * 64.f) }, { 64.f,64.f }, TexType::Tile, { 0.f,0.f });
	}
	for (int i = 25; i < 30; i++)
	{
		objs.emplace_back(rec{});
		objs[i].set({ 200.f + (19.f * 64.f),  900.f - 256.f - (((i-25) + 1) * 64.f) }, { 64.f,64.f }, TexType::Tile, { 0.f,0.f });
	}
	olc::vf2d mpos;
	float dt = 0;
	sf::Clock frameTimer;
	while (wnd.isOpen())
	{
		dt = frameTimer.restart().asSeconds();
		sf::Event e;
		while (wnd.pollEvent(e))
		{
			if (e.type == sf::Event::KeyReleased && e.key.code == sf::Keyboard::Escape)
			{
				wnd.close();
			}
		}
		if (wnd.isOpen())
		{
			mpos = olc::vf2d{ (float)sf::Mouse::getPosition(wnd).x, (float)sf::Mouse::getPosition(wnd).y };
			aRay myRay{ {objs[0].pos}, {mpos}};
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				objs[0].vel += myRay.dir() * 100.f * dt;
			}
			olc::vf2d cp;
			olc::vi2d cn;
			float ct;
			std::vector<std::pair<int, float> > z;
			for (int i = 1; i < objs.size(); i++)
			{
				rec target;
				target.set({ objs[i].pos.x, objs[i].pos.y }, { 64.f, 64.f }, TexType::NotSet, { 0.f,0.f });
				if (DynamicRectVsRect(objs[0], target, cp, cn, ct, dt))
				{
					z.push_back({ i, ct });		
				}
			}
			std::sort(z.begin(), z.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b)
				{
					return a.second < b.second;
				});
			for (auto j : z)
			{
				rec target;
				target.set({ objs[j.first].pos.x, objs[j.first].pos.y }, { 64.f, 64.f }, TexType::NotSet, { 0.f,0.f });
				if (DynamicRectVsRect(objs[0], target, cp, cn, ct, dt))
				{
					objs[0].vel += cn * olc::vf2d{std::abs(objs[0].vel.x), std::abs(objs[0].vel.y)} *(1 - ct);
				}
			}
			objs[0].pos += objs[0].vel * dt;

			wnd.clear();
			for (auto& o : objs)
			{
				wnd.draw(*spr(o));
			}
			wnd.display();
		}
	}
}