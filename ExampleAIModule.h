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
#define GT ->getTilePosition()
#define R return
#define B break
#define C case

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
	void onUnitDestroy(U u) {if (u T.isResourceDepot())ep.erase(u GT);}
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
		A gt = [](U u) {R TP{u->getTargetPosition()};};
		A sm=s->minerals(),sg=s->gas(),gp=ac(142),rg=sg>200?2:3;

		if(pb&&(!pb->exists()||pb->isMorphing()))pb=nullptr;
		for (A&u : s->getUnits()) {
			U x = u->getClosestUnit(IsEnemy&&Armor < 9);
			switch(u T) {
			C 149:
				if (u->isCompleted()) ex = u;
				B;
			C 131:
				if (!ac(132) && cb(132))u M(132);
				B;
			C 41:
				if (x && u->canAttack() && u D(x) < 33) {
					u->attack(x);
					B;
				}
				if (bo == 4 && !us&&f > 2000) {
					us = u;
					A vv = set(ep.begin(), ep.end());
					vv.erase(gt(fo));
					if (vv.size()) {
						TP r; size_t d = -1;
						for (A p : vv)if (u D(P{ p }) < d) { d = u D(P{ p }); r = p; }
						u->move(P{ r });
					}
				}
				if (u J && u != pb) {
					U r; size_t d = -1;
					for (A&m : mi)if (m D(u) < d) { d = m D(u); r = m; }
					u->gather(r);
					mi.erase(r);
				}
				if (!pb&& nc(u) && gw.find(u) == gw.end()) {
					if (!gp && sm > 191) { pb = u; tb = 142; }
					if (bo == 9) {
						if (!ac(149) && gp && sm > 41) { pb = u; tb = 149; }
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
			C 35:
				if (ac(41) < bo) u M(41);
				else if (gp && 2 + 16 * ac(42) - s->supplyUsed() < 2 && !ac(36))u M(42);
				else if (cb(43)) u M(43);
				else u M(37);
				B;
			C 42:
				if (!fo)fo=u;
				if (u J) {
					if (ep.size() > 1) {
						A vv = set(ep.begin(), ep.end());
						if (fo!=u)vv.erase(gt(fo));
						TP r; size_t d = -1;
						for (A p : vv)if (u D(P{ p }) < d) { d = u D(P{ p }); r = p; }
						u->move(P{ r });
					}
					else u->move(rm(u));
				}
				if (bv(gt(u))) ep.erase(gt(u));
				B;
			C 43:
			C 37:
				if (u J || u->getOrder() == 6) {
					TP tp;
					for (A b : ep)tp += b;
					int eps = ep.size();
					if (eps > 1)tp += s->getStartLocation();
					if (eps && g->getRegionAt(P{ tp }) != u->getRegion()) u->attack(P{ tp / eps });
					else {
						if (x)u->attack(x->getPosition());
						else {
							P r; size_t d = -1;
							for (A b : eb)if (u D(b) < d) { d = u D(b); r = b; }
							if (r.x&&r.y) {
								if (bv(TP{ r })) eb.erase(r);
								else u->attack(r);
							}
							else if (u->getOrder() != 6)u->move(rm(u));
						}
					}
				}
				B;
			}
		}
		if (us&&bv(gt(us)))ep.erase(gt(us));
		if (pb) {
			TP bl;size_t d = -1;
			int xx = pb GT.x;
			int yy = pb GT.y;
			for (int x = xx-9; x <xx+ 9; x++)for (int y = yy-9; y < yy+9; y++) {
				TP tp{ x,y };
				if (g->canBuildHere(tp, tb) && (tb == 149 || g->getClosestUnit(P{ tp }, IsMineralField)D(P{ tp }) > 200))
					if (pb D(P{ tp }) < d) { d = pb D(P{ tp }); bl = tp; }
			
			}
			if (bl) {
				if (cb(tb)) pb->build(tb, bl);
				else pb->move(P{ bl });
			}
		}		

		for (A&u:e->getUnits()) {
			if (u T.isFlyer())bo=9;
			if (u T.isBuilding())eb.insert(u->getPosition());
			if (u T.isResourceDepot() && ep.find(u GT) != ep.end())for(A t : ep) if (t != u GT) ep.erase(t);
		}
	}
};
