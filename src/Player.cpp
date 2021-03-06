#include <dodge/EventManager.hpp>
#include <dodge/math/math.hpp>
#include <dodge/EAnimFinished.hpp>
#include <dodge/WinIO.hpp>
#include <dodge/globals.hpp>
#include <dodge/renderer/Renderer.hpp>
#include <dodge/AssetManager.hpp>
#include "Player.hpp"
#include "EExplosion.hpp"
#include "ERequestToThrowThrowable.hpp"


using namespace std;
using namespace Dodge;


//===========================================
// Player::Player
//===========================================
Player::Player(const Dodge::XmlNode data)
   : Asset(Dodge::internString("Player")),
     Entity(data.firstChild().firstChild()),
     Item(data.firstChild()),
     Sprite(data.nthChild(1)),
     m_facing(UP),
     m_state(ALIVE),
     m_mode(NORMAL_MODE) {

   try {
      AssetManager assetManager;

      XML_NODE_CHECK(data, Player);

      XmlNode node = data.nthChild(2);
      XML_NODE_CHECK(node, crosshairsId);
      long crosshairsId = node.getLong();

      m_crosshairs = boost::dynamic_pointer_cast<Item>(assetManager.getAssetPointer(crosshairsId));

      if (!m_crosshairs)
         throw XmlException("Bad asset id for crosshairs", __FILE__, __LINE__);

      node = node.nextSibling();
      XML_NODE_CHECK(node, footSensor);
      m_footSensor = Quad(node.firstChild());

      node = node.nextSibling();
      XML_NODE_CHECK(node, headSensor);
      m_headSensor = Quad(node.firstChild());

      node = node.nextSibling();
      XML_NODE_CHECK(node, leftSensor);
      m_leftSensor = Quad(node.firstChild());

      node = node.nextSibling();
      XML_NODE_CHECK(node, rightSensor);
      m_rightSensor = Quad(node.firstChild());

      node = node.nextSibling();
      XML_NODE_CHECK(node, midSensor);
      m_midSensor = Quad(node.firstChild());
   }
   catch (XmlException& e) {
      e.prepend("Error parsing XML for instance of class Player; ");
      throw;
   }

   init();
}

//===========================================
// Player::Player
//===========================================
Player::Player(const Player& copy)
   : Asset(Dodge::internString("Player")),
     Entity(copy),
     Item(copy),
     Sprite(copy),
     m_state(ALIVE),
     m_mode(NORMAL_MODE) {

   deepCopy(copy);
   init();
}

//===========================================
// Player::Player
//===========================================
Player::Player(const Player& copy, long name)
   : Asset(Dodge::internString("Player")),
     Entity(copy, name),
     Item(copy, name),
     Sprite(copy, name),
     m_state(ALIVE),
     m_mode(NORMAL_MODE) {

   deepCopy(copy);
   init();
}

//===========================================
// Player::init
//===========================================
void Player::init() {
   EventManager eventManager;
   eventManager.registerCallback(internString("explosion"), Functor<void, TYPELIST_1(EEvent*)>(this, &Player::explosionHandler));

   m_originalTexSection = getTextureSection();
}

//===========================================
// Player::deepCopy
//===========================================
void Player::deepCopy(const Player& copy) {
   m_footSensor = copy.m_footSensor;
   m_headSensor = copy.m_headSensor;
   m_leftSensor = copy.m_leftSensor;
   m_rightSensor = copy.m_rightSensor;
   m_midSensor = copy.m_midSensor;
}

//===========================================
// Player::update
//===========================================
void Player::update() {
   Sprite::update();

   if (m_mode == THROWING_MODE)
      m_crosshairs->update();
}

//===========================================
// Player::draw
//===========================================
void Player::draw() const {
   Sprite::draw();

   if (m_mode == THROWING_MODE)
      m_crosshairs->draw();
}

//===========================================
// Player::getSize
//===========================================
size_t Player::getSize() const {
   return sizeof(Player)
      - sizeof(Item)
      + Item::getSize()
      - sizeof(Sprite)
      + Sprite::getSize()
      - sizeof(Asset)
      - sizeof(Quad) + m_footSensor.getSize()
      - sizeof(Quad) + m_headSensor.getSize()
      - sizeof(Quad) + m_leftSensor.getSize()
      - sizeof(Quad) + m_rightSensor.getSize();
}

//===========================================
// Player::clone
//===========================================
Asset* Player::clone() const {
   return new Player(*this);
}

#ifdef DEBUG
//===========================================
// Player::dbg_print
//===========================================
void Player::dbg_print(ostream& out, int tab) const {
   for (int i = 0; i < tab; ++i) out << "\t";

   out << "Player\n";

   Item::dbg_print(out, tab + 1);
   Sprite::dbg_print(out, tab + 1);
}
#endif

//===========================================
// Player::onEvent
//===========================================
void Player::onEvent(const EEvent* event) {
   static long entityTranslationStr = internString("entityTranslation");
   static long transPartFinishedStr = internString("transPartFinished");
   static long transFinishedStr = internString("transFinished");
   static long animFinishedStr = internString("animFinished");
   static long explodeStr = internString("explode");
   static long moveLeftStr = internString("moveLeft");
   static long moveRightStr = internString("moveRight");
   static long moveUpStr = internString("moveUp");
   static long moveDownStr = internString("moveDown");
   static long gameOverStr = internString("gameOver");
   static long hitFromAboveStr = internString("hitFromAbove");
   static long dieStr = internString("die");
   static long killedByZombieStr = internString("killedByZombie");

   Sprite::onEvent(event);

   if (event->getType() == transPartFinishedStr) {
      const ETransPartFinished* e = static_cast<const ETransPartFinished*>(event);
      pTransformation_t trans = e->transformation;

      if (trans->getName() == moveLeftStr
         || trans->getName() == moveUpStr
         || trans->getName() == moveRightStr
         || trans->getName() == moveDownStr) stepAnimation();
   }
   else if (event->getType() == transFinishedStr) {
      checkForCollisionWithThrowable();
   }
   else if (event->getType() == animFinishedStr) {
      const EAnimFinished* e = static_cast<const EAnimFinished*>(event);
      pAnimation_t anim = e->animation;

      if (anim->getName() == explodeStr || anim->getName() == dieStr) {
         EventManager eventManager;
         eventManager.queueEvent(new EEvent(gameOverStr));
      }
   }
   else if (event->getType() == entityTranslationStr) {
      checkForCollisions();
   }
   else if (event->getType() == hitFromAboveStr) { // Zombie kill
      if (m_state == ALIVE) {
         stopAnimation();
         stopTransformations();
         playAnimation(dieStr);

         EventManager eventManager;
         eventManager.queueEvent(new EEvent(killedByZombieStr));

         die();
      }
   }
}

//===========================================
// Player::explosionHandler
//===========================================
void Player::explosionHandler(EEvent* event) {
   static long explodeStr = internString("explode");

   EExplosion* e = static_cast<EExplosion*>(event);
   Vec2f pos = getTranslation_abs() + (getOnScreenSize() / 2.f);
   Vec2f diff = pos - e->pos;

   if (diff.x * diff.x + diff.y * diff.y < e->radius * e->radius) {
      if (m_state == ALIVE) {
         stopAnimation();
         stopTransformations();
         playAnimation(explodeStr);
         die();
      }
   }
}

//===========================================
// Player::die
//===========================================
void Player::die() {
   if (m_mode == THROWING_MODE)
      enterNormalMode();

   m_state = DEAD;

   EventManager eventManager;
   eventManager.queueEvent(new EEvent(internString("playerDeath")));
}

//===========================================
// Player::revive
//===========================================
void Player::revive() {
   m_state = ALIVE;
   setFillColour(Colour(1, 1, 1, 1));
   setTextureSection(m_originalTexSection);
   restoreOnScreenSize();
}

//===========================================
// Player::enterNormalMode
//===========================================
void Player::enterNormalMode() {
   m_mode = NORMAL_MODE;

   WinIO win;
   win.unregisterCallback(WinIO::EVENT_MOUSEMOVE, Functor<void, TYPELIST_2(int, int)>(this, &Player::mouseMove));
   win.unregisterCallback(WinIO::EVENT_BTN1PRESS, Functor<void, TYPELIST_2(int, int)>(this, &Player::mouseLeftClick));
   win.unregisterCallback(WinIO::EVENT_BTN3PRESS, Functor<void, TYPELIST_2(int, int)>(this, &Player::mouseRightClick));

   m_throwable.reset();
   m_crosshairs->removeFromWorld();
}

//===========================================
// Player::enterThrowingMode
//===========================================
void Player::enterThrowingMode(pThrowable_t throwable) {
   m_mode = THROWING_MODE;

   WinIO win;
   win.registerCallback(WinIO::EVENT_MOUSEMOVE, Functor<void, TYPELIST_2(int, int)>(this, &Player::mouseMove));
   win.registerCallback(WinIO::EVENT_BTN1PRESS, Functor<void, TYPELIST_2(int, int)>(this, &Player::mouseLeftClick));
   win.registerCallback(WinIO::EVENT_BTN3PRESS, Functor<void, TYPELIST_2(int, int)>(this, &Player::mouseRightClick));

   m_throwable = throwable;
   m_crosshairs->addToWorld();
}

//===========================================
// Player::mouseMove
//===========================================
void Player::mouseMove(int x, int y) {
   assert(m_mode == THROWING_MODE);

   Vec2f viewPos = Renderer::getInstance().getCamera().getTranslation();

   WinIO win;
   y = win.getWindowHeight() - y;
   float32_t wx = viewPos.x + static_cast<float32_t>(x) * gGetPixelSize().x;
   float32_t wy = viewPos.y + static_cast<float32_t>(y) * gGetPixelSize().y;

   const Vec2f& sz = m_crosshairs->getBoundary().getSize();
   m_crosshairs->setTranslation(wx - sz.x / 2.f, wy - sz.y / 2.f);
}

//===========================================
// Player::mouseLeftClick
//===========================================
void Player::mouseLeftClick(int x, int y) {
   assert(m_mode == THROWING_MODE);
   assert(m_throwable);

   Vec2f viewPos = Renderer::getInstance().getCamera().getTranslation();

   WinIO win;
   y = win.getWindowHeight() - y;
   float32_t wx = viewPos.x + static_cast<float32_t>(x) * gGetPixelSize().x;
   float32_t wy = viewPos.y + static_cast<float32_t>(y) * gGetPixelSize().y;

   EventManager eventManager;
   ERequestToThrowThrowable* event = new ERequestToThrowThrowable(m_throwable, wx, wy);
   eventManager.queueEvent(event);

   enterNormalMode();
}

//===========================================
// Player::mouseRightClick
//===========================================
void Player::mouseRightClick(int x, int y) {
   assert(m_mode == THROWING_MODE);

   enterNormalMode();
}

//===========================================
// Player::checkForCollisions
//===========================================
void Player::checkForCollisions() {
   static long hitFromRightStr = internString("hitFromRight");
   static long hitFromLeftStr = internString("hitFromLeft");
   static long hitFromBottomStr = internString("hitFromBottom");
   static long hitFromTopStr = internString("hitFromTop");
   static long hitFromAboveStr = internString("hitFromAbove");
   static long playerStandsOnStr = internString("playerStandsOn");

   vector<pEntity_t> vec;
   m_worldSpace.getEntities(getBoundary(), vec);

   for (uint_t i = 0; i < vec.size(); ++i) {
      if (vec[i].get() == this) continue;
      if (!vec[i]->hasShape()) continue;

      Vec2f A = getTranslation_abs();
      Vec2f B = vec[i]->getTranslation_abs();

      bool l = false;
      bool r = false;
      bool t = false;
      bool b = false;

      if (Math::overlap(m_leftSensor, A, vec[i]->getShape(), B)) {
         EEvent* event = new EEvent(hitFromRightStr);
         vec[i]->onEvent(event);
         delete event;

         l = true;
      }

      if (Math::overlap(m_rightSensor, A, vec[i]->getShape(), B)) {
         EEvent* event = new EEvent(hitFromLeftStr);
         vec[i]->onEvent(event);
         delete event;

         r = true;
      }

      if (Math::overlap(m_headSensor, A, vec[i]->getShape(), B)) {
         EEvent* event = new EEvent(hitFromBottomStr);
         vec[i]->onEvent(event);
         delete event;

         t = true;
      }

      if (Math::overlap(m_footSensor, A, vec[i]->getShape(), B)) {
         EEvent* event = new EEvent(hitFromTopStr);
         vec[i]->onEvent(event);
         delete event;

         b = true;
      }

      if (l && r && t && b) {
         EEvent* event = new EEvent(hitFromAboveStr);
         vec[i]->onEvent(event);
         delete event;

         EEvent* event2 = new EEvent(playerStandsOnStr);
         vec[i]->onEvent(event2);
         delete event2;
      }
   }
}

//===========================================
// Player::checkForCollisionWithThrowable
//===========================================
void Player::checkForCollisionWithThrowable() {
   static long throwableStr = internString("throwable");

   vector<pEntity_t> vec;
   m_worldSpace.getEntities(getBoundary(), vec);

   for (uint_t i = 0; i < vec.size(); ++i) {
      if (vec[i].get() == this) continue;
      if (!vec[i]->hasShape()) continue;

      Vec2f A = getTranslation_abs();
      Vec2f B = vec[i]->getTranslation_abs();

      // Check if the player has stumbled upon a throwable item
      if (m_mode != THROWING_MODE) {

         if (Math::overlap(m_midSensor, A, vec[i]->getShape(), B)) {
            if (vec[i]->getTypeName() == throwableStr) {
               pThrowable_t item = boost::dynamic_pointer_cast<Throwable>(vec[i]);
               assert(item);

               enterThrowingMode(item);
            }
         }
      }
   }
}

//===========================================
// Player::moveLeft
//===========================================
bool Player::moveLeft() {
   Vec2f pos = getTranslation_abs();
   Vec2f sz = getBoundary().getSize();

   float32_t dx = sz.x / 2.f;

   float32_t rx = m_region.getPosition().x;

   if (pos.x > rx + dx) return move(LEFT);

   return true;
}

//===========================================
// Player::moveRight
//===========================================
bool Player::moveRight() {
   Vec2f pos = getTranslation_abs();
   Vec2f sz = getBoundary().getSize();

   float32_t dx = sz.x / 2.f;

   float32_t rx = m_region.getPosition().x;
   float32_t rw = m_region.getSize().x;

   if (pos.x + sz.x < rx + rw - dx) return move(RIGHT);

   return true;
}

//===========================================
// Player::moveUp
//===========================================
bool Player::moveUp() {
   Vec2f pos = getTranslation_abs();
   Vec2f sz = getBoundary().getSize();

   float32_t dy = sz.y / 2.f;

   float32_t ry = m_region.getPosition().y;
   float32_t rh = m_region.getSize().y;

   if (pos.y + sz.y < ry + rh - dy) return move(UP);

   return true;
}

//===========================================
// Player::moveDown
//===========================================
bool Player::moveDown() {
   Vec2f pos = getTranslation_abs();
   Vec2f sz = getBoundary().getSize();

   float32_t dy = sz.y / 2.f;

   float32_t ry = m_region.getPosition().y;

   if (pos.y > ry + dy) return move(DOWN);

   return true;
}

//===========================================
// Player::move
//===========================================
bool Player::move(dir_t dir) {
   static long moveLeftStr = internString("moveLeft");
   static long moveRightStr = internString("moveRight");
   static long moveUpStr = internString("moveUp");
   static long moveDownStr = internString("moveDown");

   long anim = 0;

   switch (dir) {
      case LEFT:   anim = moveLeftStr;  break;
      case RIGHT:  anim = moveRightStr; break;
      case DOWN:   anim = moveDownStr;  break;
      case UP:     anim = moveUpStr;    break;
   }

   playAnimation(anim, true);
   pauseAnimation();

   if (numActiveTransformations() == 0) {
      if (m_mode == THROWING_MODE) enterNormalMode();

      playTransformation(anim);
      m_facing = dir;

      return true;
   }

   return false;
}

//===========================================
// Player::assignData
//
// I'm not using this function
//===========================================
void Player::assignData(const Dodge::XmlNode data) {
   assert(false);
}

//===========================================
// Player::~Player
//===========================================
Player::~Player() {
   EventManager eventManager;
   eventManager.unregisterCallback(internString("explosion"), Functor<void, TYPELIST_1(EEvent*)>(this, &Player::explosionHandler));

   WinIO win;
   win.unregisterCallback(WinIO::EVENT_MOUSEMOVE, Functor<void, TYPELIST_2(int, int)>(this, &Player::mouseMove));
   win.unregisterCallback(WinIO::EVENT_BTN1PRESS, Functor<void, TYPELIST_2(int, int)>(this, &Player::mouseLeftClick));
   win.unregisterCallback(WinIO::EVENT_BTN3PRESS, Functor<void, TYPELIST_2(int, int)>(this, &Player::mouseRightClick));
}
