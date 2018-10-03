#include <BWAPI.h>
#include <set>
#define S using
#define N S namespace
#define A auto
#define D ->getDistance
#define J ->isIdle()
#define M ->morph
#define T ->getType()

N BWAPI;
N Filter;
N std;
N UnitTypes;
S U = Unit;
S TP = TilePosition;
S P = Position;
A&g = BroodwarPtr;

U ex; //extractor
U pb; //builder
UnitType tb; //what to build
int bo=4; //buildorder, default 4pool, if zerg 9pool->muta, if detect flying enemy->4=9

set<TP>ep; //enemy positions
set<U>gw; //gas workers
set<P>eb; //enemybuildings
//scouting with drones
//todo scout all mineralplaces for buildings
//todo find way to attack static defense with wining army numbers
//todo send overlords to better places


struct ExampleAIModule:AIModule {
	void onUnitDestroy(U u) {
		if (u T.isResourceDepot())ep.erase(u->getTilePosition());
	}
	void onFrame() {
		string debugstring = "";
		A s = g->self();
		A e = g->enemy();
		if (!g->getFrameCount()) {
			//g->sendText("black sheep wall");
			for (A b : g->getStartLocations()) ep.insert(b);
			ep.erase(s->getStartLocation());
			if (e->getRace() == Races::Zerg) bo = 9;
		}
		A mi = g->getStaticMinerals();
		A ac = [&](int t) {return s->allUnitCount(t); };
		A cb = [](int t) {return g->canMake(t);};
		A nc = [](U u) {return !(u->isCarryingMinerals() || u->isCarryingGas()); };
		A rm = [&](U u) {return vector(mi.begin(), mi.end())[(int)u%mi.size()]->getPosition();};
		A bv = [](TP p) {return g->isVisible(p) && !g->getUnitsOnTile(p, IsEnemy && IsBuilding).size();};

		A sm = s->minerals(), sg = s->gas(), gp = ac(Zerg_Spawning_Pool),rg=sg>200?2:3;

		if (pb && (!pb->exists() || pb->isMorphing())) pb = nullptr;
		for (A&u : s->getUnits()){
			U x = u->getClosestUnit(IsEnemy); //add not ==larva/egg
			switch (u T) {
			case Zerg_Extractor:
				if (u->isCompleted()) ex = u;
				break;
			case Zerg_Hatchery:
				if (!ac(Zerg_Lair) && cb(Zerg_Lair)) u M(Zerg_Lair);
				break;
			case Zerg_Drone:
				//worker defense
				if (x && u->canAttack() && u->getDistance(x) < 33) {
					u->attack(x);
					break;
				}
				// mine minerals
				if (u J && u != pb) {
					U r; size_t d = -1;
					for (A&m : mi)if (m D(u) < d) { d = m D(u); r = m; }
					u->gather(r);
					mi.erase(r);
				}
				//check if we want to build a building
				if (!pb && nc(u) && gw.find(u)==gw.end()) {
					if (!gp && sm > 191) {
						pb = u;
						tb = Zerg_Spawning_Pool;
					}
					if (bo == 9) {
						if (!ac(Zerg_Extractor) && gp && sm > 41) {
							pb = u;
							tb = Zerg_Extractor;
						}
						else if (!ac(Zerg_Spire) && cb(Zerg_Spire)) {
							pb = u;
							tb = Zerg_Spire;
						}
						else if (sm > 450) {
							pb = u;
							tb = Zerg_Hatchery;
						}
					}
				}
				// mine gas
				if (ex && nc(u)) {
					if (gw.size() < rg) {
						gw.insert(u);
						u->gather(ex);
					}
					if (gw.size() > rg && gw.find(u) != gw.end()) {
						gw.erase(u);
						u->stop();
					}
				}
				break;
			case Zerg_Larva:
				if (ac(Zerg_Drone) < bo) u M(Zerg_Drone);
				else if (gp && 2 + 16 * ac(Zerg_Overlord) - s->supplyUsed() < 2 && !ac(Zerg_Egg)) u M(Zerg_Overlord);
				else if (cb(Zerg_Mutalisk)) u M(Zerg_Mutalisk);
				else u M(Zerg_Zergling);
				break;
			case Zerg_Overlord:
				//todo run away with overlord if under attack
				if (u J) {
					if (ep.size() > 1) {
						TP r; size_t d = -1;
						for (A p : ep)if (u D(P{ p }) < d) { d = u D(P{ p }); r = p; }
						u->move(P{ r });
					}
					else u->move(rm(u));
				}
				{A p = TP{ u->getTargetPosition() };
				if (bv(p)) ep.erase(p); }
				break;
			case Zerg_Mutalisk:
			case Zerg_Zergling:
				if (u J || u->getOrder() == Orders::Move) {
					TP tp;
					for (A b : ep)tp += b;
					int eps = ep.size();
					if (eps > 1)tp += s->getStartLocation();
					if (eps && g->getRegionAt(P{ tp }) != u->getRegion()) u->attack(P{tp/ eps });
					else {
						if (x)u->attack(x->getPosition());
						else {
							//attack closest building
							P r; size_t d = -1;
							for (A b : eb)if (u D(b) < d) { d = u D(b); r = b; }
							if (r.x&&r.y) {
								if (bv(TP{r})) eb.erase(r);
								else u->attack(r);
							}
							//choose random mineral
							else if (u->getOrder() != Orders::Move)u->move(rm(u));
						}
					}
				}
				break;
			}
		}
		debugstring += "eb: " + to_string(eb.size());
		
		//build
		if (pb) {
			TP bl;size_t d = -1;
			//todo blacklist to close to own minerals
			int xx = pb->getTilePosition().x;
			int yy = pb->getTilePosition().y;
			for (int x = xx-9; x <xx+ 9; x++)for (int y = yy-9; y < yy+9; y++) {
				TP tp{ x,y };
				if (g->canBuildHere(tp, tb)){
					g->drawBoxMap(x * 32, y * 32, x * 32 + 32, y * 32 + 32, Colors::Green);
					if(pb D(P{tp})<d){d=pb D(P{tp});bl=tp;}
				}
			}
			g->drawCircleMap(Position{ bl }, 5, Colors::Red,1);
			g->drawCircleMap(pb->getPosition(), 5, Colors::Red, 1);
			g->drawLineMap(Position{ bl }, pb->getPosition(), Colors::Yellow);
			if (cb(tb)) pb->build(tb, bl);
			else pb->move(P{ bl });
		}		

		for (A&u : e->getUnits()) {
			if (u T.isFlyer()) {
				bo = 9;
			}	
			if (u T.isBuilding()) {
				eb.insert(u->getPosition());
			}
			if (u T.isResourceDepot()) {
				TP tp = u->getTilePosition();
				if (ep.find(tp) != ep.end())for(TP t : ep) if (t != tp) ep.erase(t);
			}
		}
		g->drawTextScreen(50, 50, debugstring.c_str());
	}
};
