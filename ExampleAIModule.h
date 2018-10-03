#include <BWAPI.h>
#include <set>
#define N using namespace
#define D ->getDistance
#define J ->isIdle()
#define M ->morph
#define TP TilePosition
#define P Position
#define T ->getType()

N BWAPI;
N Filter;
N std;
N UnitTypes;
using U = Unit;
auto&g = BroodwarPtr;

U ex; //extractor
U pb; //builder
UnitType tb; //what to build
int bo=4; //buildorder, default 4pool, if zerg 9pool->muta, if detect flying enemy->4=9

set<TP>ep; //enemy positions
set<U>gw; //gas workers
set<U>eb; //enemybuildings
//scouting
//todo scout all mineralplaces for buildings


struct ExampleAIModule:AIModule {
	void onUnitDestroy(U u) {
		eb.erase(u);
		if (u T.isResourceDepot())ep.erase(u->getTilePosition());
	}
	void onFrame() {
		string debugstring = "";
		auto s = g->self();

		if (!g->getFrameCount()) {
			g->sendText("black sheep wall");
			for (auto b : g->getStartLocations()) ep.insert(b);
			ep.erase(s->getStartLocation());
		}
		auto e = g->enemy();
		auto mi = g->getStaticMinerals();
		auto auc = [s](int t) {return s->allUnitCount(t); };
		auto cb = [](int t) {return g->canMake(t);};
		auto notcarry = [](U u) {return !(u->isCarryingMinerals() || u->isCarryingGas()); };
		auto sm = s->minerals(), sg = s->gas(), gp = auc(Zerg_Spawning_Pool),rg=sg>200?2:3;

		if (pb && (!pb->exists() || pb->isMorphing())) pb = nullptr;
		for (auto&u : s->getUnits()){
			U x = u->getClosestUnit(IsEnemy && CanAttack); //add not ==larva/egg
			switch (u->getType()) {
			case Zerg_Extractor:
				if (u->isCompleted()) ex = u;
				break;
			case Zerg_Hatchery:
				if (!auc(Zerg_Lair) && cb(Zerg_Lair)) u M(Zerg_Lair);
				break;
			case Zerg_Drone:
				//worker defense
				if (x && u->getDistance(x) < 33) {
					u->attack(x);
					break;
				}
				// mine minerals
				if (u J && u != pb) {
					U r; size_t d = -1;
					for (auto&m : mi)if (m D(u) < d) { d = m D(u); r = m; }
					u->gather(r);
					mi.erase(r);
				}
				//check if we want to build a building
				if (!pb && notcarry(u) && gw.find(u)==gw.end()) {
					if (!gp && sm > 191) {
						pb = u;
						tb = Zerg_Spawning_Pool;
					}
					if (bo == 9) {
						if (!auc(Zerg_Extractor) && gp && sm > 41) {
							pb = u;
							tb = Zerg_Extractor;
						}
						else if (!auc(Zerg_Spire) && cb(Zerg_Spire)) {
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
				if (ex && notcarry(u)) {
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
				if (auc(Zerg_Drone) < bo) u M(Zerg_Drone);
				else if (gp && 2 + 16 * auc(Zerg_Overlord) - s->supplyUsed() < 2 && !auc(Zerg_Egg)) u M(Zerg_Overlord);
				else if (cb(Zerg_Mutalisk)) u M(Zerg_Mutalisk);
				else u M(Zerg_Zergling);
				break;
			case Zerg_Overlord:
				//if ()
				break;
			case Zerg_Mutalisk:
			case Zerg_Zergling:
				TP tp;
				for (auto b : ep)tp += b;
				tp = tp / ep.size();
				Region r = g->getRegionAt(P{ tp });
				if (u J || u->getOrder() == Orders::Move) {
					if (r != u->getRegion()) u->attack(P{ tp });
					else {
						//todo maybe ignore larvea/egg
						if (x)u->attack(x);
						else {
							//TODO
							//if (eb.begin() != eb.end()) u->attack(eb[eb.begin()]);
							//choose random mineral
							//else if (u->getOrder() != Orders::Move)u->move(mi[rand() % mi.size()]);
						}
					}
				}
				break;
			}
		}
		//debugstring = "" + to_string(s->supplyUsed()) + " - " + to_string(2 + 16 * auc(Zerg_Overlord));
		debugstring += "eb: " + to_string(eb.size());
		
		//build
		if (pb) {
			TP bl;size_t d = -1;
			//todo blacklist to close to own minerals
			for (int x = 0; x < g->mapWidth(); x++)for (int y = 0; y < g->mapHeight(); y++) {
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

		for (auto&u : e->getUnits()) {
			if (u T.isFlyer()) {
				bo = 9;
			}	
			if (u T.isBuilding()) {
				eb.insert(u);
			}
			if (u T.isResourceDepot()) {
				TP tp = u->getTilePosition();
				if (ep.find(tp) != ep.end())for(TP t : ep) if (t != tp) ep.erase(t);
				debugstring += "-- (" + to_string(tp.x) + ", " + to_string(tp.y) + ") --";
			}
		}
		debugstring += " - ep: ";
		for (TP t: ep) 
			debugstring += "(" + to_string(t.x) + ", " + to_string(t.y) + ") ";

		g->drawTextScreen(50, 50, debugstring.c_str());
	}
};
