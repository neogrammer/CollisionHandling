#include <SFML/Graphics.hpp>
#include  <olcUTIL_Geometry2D.h>
#include <vector>
using namespace olc::utils::geom2d;

struct rec
{
	olc::vf2d pos;
	olc::vf2d size;
};

struct aRay
{
	olc::vf2d origin;
	olc::vf2d endPt;
	aRay(olc::vf2d start,  olc::vf2d end)
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
		return { D().x / D().mag(), D().y / D().mag() };
	}
	
};

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

int main()
{

	


	sf::RenderWindow wnd{ {1600U,900U,32U}, "Collisions" };


	sf::Texture tileTex;
	tileTex.loadFromFile("assets/tileset.png");
	std::vector<sf::Sprite> tiles;
	tiles.clear();
	for (int i = 0; i < 20; i++)
	{
		tiles.emplace_back(sf::Sprite{});
		tiles.back().setPosition({ ((float)i  * 64.f) + 200.f, 900.f - 64.f });
		tiles.back().setTextureRect({ {0, 0},{64,64} });
		tiles.back().setTexture(tileTex);
	}
	sf::Vector2i iMouse;
	olc::vf2d mpos;
	
	

	while (wnd.isOpen())
	{
		

	

	
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
			iMouse = sf::Mouse::getPosition(wnd);
			mpos = olc::vf2d{ (float)iMouse.x, (float)iMouse.y };
			rec r = { { 100.f*4, 100.f*4}, {50.f*4, 30.f*4} };
			aRay myRay{ {20.f,20.f}, {mpos} };
			
			olc::vf2d cp;
			olc::vi2d cn;
			float t;

			sf::Color col = sf::Color::White;

			bool collided = false;
			if (RayVsRect(myRay.orig(), myRay.D(), r, cp, cn, t) && t < 1.0f)
			{
				col = sf::Color::Yellow;
				collided = true;

			}
			else
			{
				col = sf::Color::White;
			}

			wnd.clear();

			for (auto& t : tiles)
			{
				wnd.draw(t);
			}

			sf::RectangleShape shp({ r.size.x, r.size.y});
			shp.setOutlineColor(col);
			shp.setPosition({ r.pos.x, r.pos.y});
			shp.setOutlineThickness(2);
			shp.setFillColor(sf::Color::Transparent);

			wnd.draw(shp);

			if (collided)
			{
				sf::CircleShape cShp{ 12.f };
				cShp.setFillColor(sf::Color::Red);
				cShp.setOutlineColor(sf::Color::Transparent);
				cShp.setPosition({cp.x, cp.y});
				cShp.setOrigin({ cShp.getRadius() + shp.getOutlineThickness() / 2.f, cShp.getRadius() + shp.getOutlineThickness() / 2.f});
				wnd.draw(cShp);

				sf::RectangleShape nshp({(cn.x != 0) ? 40.f * cn.x :  0.f, (cn.y != 0) ? 40.f * cn.y : 0.f });
				nshp.setOutlineColor(sf::Color::White);
			   nshp.setPosition({ cp.x,cp.y });
				nshp.setOutlineThickness(2);
				nshp.setFillColor(sf::Color::Transparent);
				wnd.draw(nshp);
			}


			sf::VertexArray lShp{ sf::PrimitiveType::Lines, 2Ui64 };
			lShp[0].position = { myRay.orig().x,   myRay.orig().y };
			lShp[0].color = sf::Color::White;
			lShp[0].position = { myRay.endPt.x, myRay.endPt.y };
			lShp[0].color = sf::Color::White;

			wnd.draw(lShp);

			//sf::RectangleShape shp2({ s.size.x, s.size.y });
			//shp2.setOutlineColor(sf::Color::Green);
			//shp2.setPosition({ s.pos.x, s.pos.y });
			//shp2.setOutlineThickness(2);
			//shp2.setFillColor(sf::Color::Transparent);

			//wnd.draw(shp2);

			wnd.display();

		}
	}
}