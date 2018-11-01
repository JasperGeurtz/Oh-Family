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
#define K ->attack
#define R return
#define B break
#define C case
#define Q erase
#define I if
#define E else
#define F for
#define W insert
#define Z size()
#define Y end()
N BWAPI;N Filter;N std;S U=Unit;S TP=TilePosition;S P=Position;A&g=BroodwarPtr;U fo;U us;U ex;U pb;UnitType tb;A bo=4;set<TP>ep;set<U>gw;set<P>eb;
struct ExampleAIModule:AIModule {
void onUnitDestroy(U u){I(u T.RD)ep.Q(u GT);}
void onFrame() {
A s=g->self();
A e=g->enemy();
A f=g->getFrameCount();
I(!f) {F(A b:g->getStartLocations())ep.W(b);
ep.Q(s->getStartLocation());
I(!e->getRace())bo=9;}
A mi=g->getStaticMinerals();A ac=[&](A t){R s->allUnitCount(t);};A cb=[](A t){R g->canMake(t);};
A rm=[&](U u){R vector(mi.begin(),mi.Y)[time(0)%mi.Z]->getInitialPosition();};
A bv=[](TP p){R g->isVisible(p) && !g->getUnitsOnTile(p,IsEnemy&&IsBuilding).Z;};
A gt=[](U u){R TP{u->getTargetPosition()};};
A sm=s->minerals(),sg=s->gas(),gp=ac(142),rg=sg>200?2:3;
size_t st=-1;
I(pb&&(!pb->exists()||pb->isMorphing()))pb=nullptr;
F(A&u:s->getUnits()) {
U x=u->getClosestUnit(IsEnemy&&Armor<9);
switch(u T){
C 149:I (u->isCompleted()) ex = u;B;
C 131:I (!ac(132) && cb(132))u M(132);B;
C 41:I(x&&u->canAttack()&&u D(x)<33){u K(x);B;}
I(bo==4&&!us&&f>2000) {
us=u;A vv=set(ep.begin(),ep.Y);
vv.Q(gt(fo));
I(vv.Z){
TP r;A d=st;
F(A p:vv)I(u D(P{p})<d){d=u D(P{p});r=p;}
u->move(P{r});}}
I(u J&&u!=pb){U r;A d=st;
F(A&m:mi)I(m D(u)<d){d=m D(u);r=m;}
u->gather(r);
mi.Q(r);}
I(!pb&&gw.find(u)==gw.Y){
I(!gp&&sm>191){pb=u;tb=142;}
I(bo==9) {
I(!ac(149)&&gp&&sm>41){pb=u;tb=149;}
E I(!ac(141)&&cb(141)){pb=u;tb=141;}
E I(sm>450) {
pb=u;tb=131;}}}
I(ex) {
I(gw.Z < rg) {
gw.W(u);
u->gather(ex);}
I(gw.Z>rg&&gw.find(u)!=gw.Y){
gw.Q(u);u->stop();}}B;
C 35:I(ac(41) < bo)u M(41);
E I(gp&&2+16*ac(42)-s->supplyUsed()<2&&!ac(36))u M(42);
E I(cb(43))u M(43);
E u M(37);B;
C 42:I(!fo)fo=u;I(u J){I(ep.Z>1){
A vv=set(ep.begin(), ep.Y);
I(fo!=u)vv.Q(gt(fo));
TP r;A d=st;F(A p:vv)I(u D(P{p})<d){d=u D(P{p});r=p;}
u->move(P{ r });}
E u->move(rm(u));}
I(bv(gt(u))) ep.Q(gt(u));B;
C 43:C 37:A z=u->getOrder()==6;
I(u J||z)){
TP tp;F(A b:ep)tp+=b;
A eps=ep.Z;
I(eps>1)tp+=s->getStartLocation();
I(eps&&g->getRegionAt(P{tp})!=u->getRegion())u K(P{tp/eps});
E {I(x)u K(x->getPosition());
E {P r;A d=st;
F(A b:eb)I(u D(b)<d){d=u D(b);r=b;}
I(r.x&&r.y) {
I(bv(TP{ r })) eb.Q(r);
E u K(r);}
E I(!z)u->move(rm(u));}}}B;}}
I(us&&bv(gt(us)))ep.Q(gt(us));
I(pb){TP bl;A d=st;A xx=pb GT.x;A yy=pb GT.y;
F(A x=xx-9;x<xx+9;x++)F(A y=yy-9;y<yy+9;y++) {
TP tp{x,y};
I(g->canBuildHere(tp,tb)&&(tb==149||g->getClosestUnit(P{tp}, IsMineralField)D(P{tp})>200))
I(pb D(P{tp})<d){d=pb D(P{tp});bl=tp;}}
I(bl) {
I(cb(tb))pb->build(tb,bl);
E pb->move(P{bl});}}		
F(A&u:e->getUnits()){
I(u T.isFlyer())bo=9;
I(u T.isBuilding())eb.W(u->getPosition());
I(u T.RD&&ep.find(u GT)!=ep.Y)F(A t:ep)I(t!=u GT)ep.Q(t);}}};