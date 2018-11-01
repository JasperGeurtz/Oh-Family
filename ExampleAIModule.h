#include <BWAPI.h>
#include <set>
#include<fstream>
#include <time.h>
#define S using
#define N S namespace
#define A auto
#define D ->getDistance
#define J ->isIdle()
#define M ->morph
#define T ->getType()
#define R return
#define B break

N BWAPI;
N Filter;
N std;
S U = Unit;
S TP = TilePosition;
S P = Position;
A&g = BroodwarPtr;

U fo;U us;U ex;U pb;
UnitType tb;
int bo=4; //buildorder

set<TP>ep;
set<U>gw;
set<P>eb;
//todo find way to attack static defense with wining army numbers

struct ExampleAIModule:AIModule {
	void onUnitDestroy(U u) {
		if (u T.isResourceDepot())ep.erase(u->getTilePosition());
	}
	void onFrame() {
		A s=g->self();
		A e=g->enemy();
		A f = g->getFrameCount();
		if (!f) {
			for(A b : g->getStartLocations())ep.insert(b);
			ep.erase(s->getStartLocation());
			if (!e->getRace())bo=9;
		}
		A mi=g->getStaticMinerals();
		A ac=[&](int t) {R s->allUnitCount(t); };
		A cb=[](int t) {R g->canMake(t);};
		A nc=[](U u) {R !(u->isCarryingMinerals() || u->isCarryingGas()); };
		A rm=[&](U u) {R vector(mi.begin(), mi.end())[time(0)%mi.size()]->getInitialPosition();};
		A bv=[](TP p) {R g->isVisible(p) && !g->getUnitsOnTile(p, IsEnemy && IsBuilding).size();};
		A sm=s->minerals(),sg=s->gas(),gp=ac(142),rg=sg>200?2:3;

		if(pb&&(!pb->exists()||pb->isMorphing()))pb=nullptr;
		for (A&u : s->getUnits()) {
			U x = u->getClosestUnit(IsEnemy&&Armor<9);
			switch (u T) {
			case 149:
				if (u->isCompleted()) ex = u;
				B;
			case 131:
				if(!ac(132)&&cb(132))u M(132);
				B;
			case 41:
				//worker defense
				if (x && u->canAttack() && u D(x) < 33) {
					u->attack(x);
					B;
				}
				if(bo==4&&!us&&f>2000) {
					us=u;
					A vv = set(ep.begin(), ep.end());
					vv.erase(TP{fo->getTargetPosition()});
					if (vv.size()) {
						TP r; size_t d = -1;
						for (A p : vv)if (u D(P{ p }) < d) { d = u D(P{ p }); r = p; }
						u->move(P{ r });
					}
				}
				// mine minerals
				if(u J && u != pb) {
					U r; size_t d = -1;
					for (A&m : mi)if (m D(u) < d) { d = m D(u); r = m; }
					u->gather(r);
					mi.erase(r);
				}
				//check if we want to build a building
				if (!pb && nc(u) && gw.find(u)==gw.end()) {
					if(!gp && sm > 191) {
						pb = u;
						tb = 142;
					}
					if(bo==9) {
						if (!ac(149) && gp && sm > 41) {
							pb = u;
							tb = 149;
						}
						else if (!ac(141) && cb(141)) {
							pb = u;
							tb = 141;
						}
						else if (sm > 450) {
							pb = u;
							tb = 131;
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
				B;
			case 35:
				if (ac(41) < bo) u M(41);
				else if (gp&&2+16*ac(42)-s->supplyUsed()<2&&!ac(36))u M(42);
				else if (cb(43)) u M(43);
				else u M(37);
				B;
			case 42:
				if(!fo)fo=u;
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
				B;
			case 43:
			case 37:
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
				B;
			}
		}		
		//build
		if (pb) {
			TP bl;size_t d = -1;
			int xx = pb->getTilePosition().x;
			int yy = pb->getTilePosition().y;
			for (int x = xx-9; x <xx+ 9; x++)for (int y = yy-9; y < yy+9; y++) {
				TP tp{ x,y };
				if (g->canBuildHere(tp, tb) && g->getClosestUnit(P{ tp }, IsMineralField)D(P{ tp }) > 200)
					if(pb D(P{tp})<d){d=pb D(P{tp});bl=tp;}
			}
			if (bl) {
				if (cb(tb)) pb->build(tb, bl);
				else pb->move(P{ bl });
			}
		}		

		for (A&u:e->getUnits()) {
			if (u T.isFlyer())bo=9;
			if (u T.isBuilding())eb.insert(u->getPosition());
			if (u T.isResourceDepot()) {
				TP tp = u->getTilePosition();
				if (ep.find(tp) != ep.end())for(A t : ep) if (t != tp) ep.erase(t);
			}
		}
	}
};
