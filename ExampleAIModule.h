#include <BWAPI.h>
#include <set>
#include <time.h>
#define S using
#define N S namespace
#define A auto
#define D ->getDistance
#define J ->isIdle()
#define M ->morph
#define T ->getType()
#define GT ->getTilePosition()
#define RD isResourceDepot()
#define R return
#define B break
#define C case
#define Q erase
#define I if
#define E else
N BWAPI;
N Filter;
N std;
S U = Unit;
S TP = TilePosition;
S P = Position;
A&g = BroodwarPtr;
U fo;U us;U ex;U pb;UnitType tb;int bo=4;
set<TP>ep;set<U>gw;set<P>eb;
struct ExampleAIModule:AIModule {
	void onUnitDestroy(U u){I(u T.RD)ep.Q(u GT);}
	void onFrame() {
		A s=g->self();
		A e=g->enemy();
		A f=g->getFrameCount();
		I(!f) {
			for(A b:g->getStartLocations())ep.insert(b);
			ep.Q(s->getStartLocation());
			I (!e->getRace())bo=9;
		}
		A mi=g->getStaticMinerals();
		A ac=[&](int t) {R s->allUnitCount(t); };
		A cb=[](int t) {R g->canMake(t);};
		A nc=[](U u) {R !(u->isCarryingMinerals() || u->isCarryingGas()); };
		A rm=[&](U u) {R vector(mi.begin(), mi.end())[time(0)%mi.size()]->getInitialPosition();};
		A bv=[](TP p) {R g->isVisible(p) && !g->getUnitsOnTile(p, IsEnemy && IsBuilding).size();};
		A gt = [](U u) {R TP{u->getTargetPosition()};};
		A sm=s->minerals(),sg=s->gas(),gp=ac(142),rg=sg>200?2:3;
		I(pb&&(!pb->exists()||pb->isMorphing()))pb=nullptr;
		for (A&u : s->getUnits()) {
			U x = u->getClosestUnit(IsEnemy&&Armor < 9);
			switch(u T) {
			C 149:
			I (u->isCompleted()) ex = u;B;
			C 131:
			I (!ac(132) && cb(132))u M(132);B;
			C 41:
			I(x && u->canAttack() && u D(x) < 33) {u->attack(x);B;}
			I(bo == 4 && !us&&f > 2000) {
				us = u;
				A vv = set(ep.begin(), ep.end());
				vv.Q(gt(fo));
				I(vv.size()) {
					TP r; size_t d = -1;
					for (A p : vv)I(u D(P{ p }) < d) { d = u D(P{ p }); r = p; }
					u->move(P{ r });
				}
			}
			I(u J && u != pb) {
				U r; size_t d = -1;
				for (A&m : mi)I(m D(u) < d) { d = m D(u); r = m; }
				u->gather(r);
				mi.Q(r);
			}
			I(!pb&& nc(u) && gw.find(u) == gw.end()) {
				I(!gp && sm > 191) { pb = u; tb = 142; }
				I(bo == 9) {
					I(!ac(149) && gp && sm > 41) { pb = u; tb = 149; }
						E I(!ac(141) && cb(141)) {
							pb = u;
							tb = 141;
						}
						E I(sm > 450) {
							pb = u;
							tb = 131;
						}
					}
				}
			I(ex && nc(u)) {
				I(gw.size() < rg) {
						gw.insert(u);
						u->gather(ex);
					}
					I (gw.size() > rg && gw.find(u) != gw.end()) {
						gw.Q(u);
						u->stop();
					}
				}
				B;
			C 35:
			I(ac(41) < bo) u M(41);
				E I(gp && 2 + 16 * ac(42) - s->supplyUsed() < 2 && !ac(36))u M(42);
				E I(cb(43)) u M(43);
				E u M(37);B;
			C 42:
			I(!fo)fo=u;
			I(u J) {
				I(ep.size() > 1) {
						A vv = set(ep.begin(), ep.end());
						I(fo!=u)vv.Q(gt(fo));
						TP r; size_t d = -1;
						for (A p : vv)I(u D(P{ p }) < d) { d = u D(P{ p }); r = p; }
						u->move(P{ r });
					}
					E u->move(rm(u));
				}
			I(bv(gt(u))) ep.Q(gt(u));B;
			C 43:
			C 37:
				A z=u->getOrder()==6;
				I(u J ||z) {
					TP tp;
					for (A b : ep)tp += b;
					int eps = ep.size();
					I(eps > 1)tp += s->getStartLocation();
					I(eps && g->getRegionAt(P{ tp }) != u->getRegion()) u->attack(P{ tp / eps });
					E {
					I(x)u->attack(x->getPosition());
						E {
							P r; size_t d = -1;
							for (A b : eb)I(u D(b) < d) { d = u D(b); r = b; }
							I(r.x&&r.y) {
								I(bv(TP{ r })) eb.Q(r);
								E u->attack(r);
							}
							E I(!z)u->move(rm(u));
						}
					}
				}B;
			}
		}
		I(us&&bv(gt(us)))ep.Q(gt(us));
		I(pb) {
			TP bl;size_t d = -1;
			int xx = pb GT.x;
			int yy = pb GT.y;
			for (int x = xx-9; x <xx+ 9; x++)for (int y = yy-9; y < yy+9; y++) {
				TP tp{ x,y };
				I(g->canBuildHere(tp, tb) && (tb == 149 || g->getClosestUnit(P{ tp }, IsMineralField)D(P{ tp }) > 200))
					I(pb D(P{ tp }) < d) { d = pb D(P{ tp }); bl = tp; }
			
			}
			I(bl) {
				I(cb(tb)) pb->build(tb, bl);
				E pb->move(P{ bl });
			}
		}		
		for (A&u:e->getUnits()) {
			I(u T.isFlyer())bo=9;
			I(u T.isBuilding())eb.insert(u->getPosition());
			I(u T.RD&&ep.find(u GT)!=ep.end())for(A t:ep)I(t!=u GT)ep.Q(t);
		}
	}
};