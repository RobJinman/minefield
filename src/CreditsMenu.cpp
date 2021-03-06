#include <fstream>
#include <dodge/StringId.hpp>
#include <dodge/EAnimFinished.hpp>
#include <dodge/EventManager.hpp>
#include <dodge/AssetManager.hpp>
#include <dodge/TextEntity.hpp>
#include <dodge/globals.hpp>
#include "CreditsMenu.hpp"


using namespace std;
using namespace Dodge;


//===========================================
// CreditsMenu::CreditsMenu
//===========================================
CreditsMenu::CreditsMenu(const XmlNode data)
   : Asset(internString("CreditsMenu")),
     Entity(data.firstChild().firstChild().firstChild()),
     Menu(data.firstChild()) {

   try {
      AssetManager assetManager;

      XML_NODE_CHECK(data, CreditsMenu);

      XmlNode node = data.nthChild(1);
      XML_NODE_CHECK(node, font);

      XmlAttribute attr = node.firstAttribute();
      XML_ATTR_CHECK(attr, ptr);

      long id = attr.getLong();
      m_font = boost::dynamic_pointer_cast<Dodge::Font>(assetManager.getAssetPointer(id));

      if (!m_font)
         throw XmlException("Bad font asset id", __FILE__, __LINE__);

      node = node.nextSibling();
      XML_NODE_CHECK(node, fadeInTime);
      m_fadeInTime = node.getFloat();
   }
   catch (XmlException& e) {
      e.prepend("Error parsing XML for instance of class CreditsMenu; ");
      throw;
   }

   init();
}

//===========================================
// CreditsMenu::CreditsMenu
//===========================================
CreditsMenu::CreditsMenu(const CreditsMenu& copy)
   : Asset(internString("CreditsMenu")),
     Entity(copy),
     Menu(copy) { init(); }

//===========================================
// CreditsMenu::CreditsMenu
//===========================================
CreditsMenu::CreditsMenu(const CreditsMenu& copy, long name)
   : Asset(internString("CreditsMenu")),
     Entity(copy, name),
     Menu(copy, name) { init(); }

//===========================================
// CreditsMenu::getVersionNumber
//===========================================
string CreditsMenu::getVersionNumber() const {
   ifstream fin(gGetWorkingDir() + "/data/VERSION");

   if (!fin.good()) {
      fin.close();
      return "";
   }

   string version;
   fin >> version;

   fin.close();

   return version;
}

//===========================================
// CreditsMenu::init
//===========================================
void CreditsMenu::init() {
   string version = getVersionNumber();

   pTextEntity_t txt1(new TextEntity(internString("text"), m_font, "Design & Programming: Rob Jinman", Vec2f(0.022f, 0.044f)));
   pTextEntity_t txt2(new TextEntity(internString("text"), m_font, "Music:                Jack Normal", Vec2f(0.022f, 0.044f)));
   pTextEntity_t txt3(new TextEntity(internString("text"), m_font, "Sprites: http://untamed.wild-refuge.net", Vec2f(0.02f, 0.04f)));
   pTextEntity_t txt4(new TextEntity(internString("text"), m_font, version, Vec2f(0.0175f, 0.035f)));

   m_textEntities.push_back(txt1);
   m_textEntities.push_back(txt2);
   m_textEntities.push_back(txt3);
   m_textEntities.push_back(txt4);

   addChild(txt1);
   addChild(txt2);
   addChild(txt3);
   addChild(txt4);

   txt1->setFillColour(Colour(0.f, 0.f, 0.f, 0.f));
   txt1->setTranslation(0.54f, 0.7f);
   txt1->setZ(9.f);

   txt2->setFillColour(Colour(0.f, 0.f, 0.f, 0.f));
   txt2->setTranslation(0.54f, 0.65f);
   txt2->setZ(9.f);

   txt3->setFillColour(Colour(0.5f, 0.5f, 0.5f, 0.f));
   txt3->setTranslation(0.5f, 0.02f);
   txt3->setZ(9.f);

   txt4->setFillColour(Colour(0.7f, 0.7f, 0.7f, 0.f));
   txt4->setTranslation(0.01f, 0.96f);
   txt4->setZ(9.f);
}

//===========================================
// CreditsMenu::onMenuItemActivate
//===========================================
void CreditsMenu::onMenuItemActivate(pMenuItem_t item) {
   static long mnuBackToStartMenuStr = internString("mnuCreditsToStartMenu");

   if (item->getName() == mnuBackToStartMenuStr)
      returnToParentMenu();
}

//===========================================
// CreditsMenu::getSize
//===========================================
size_t CreditsMenu::getSize() const {
   return sizeof(CreditsMenu)
      - sizeof(Menu)
      + Menu::getSize();
}

//===========================================
// CreditsMenu::clone
//===========================================
Asset* CreditsMenu::clone() const {
   return new CreditsMenu(*this);
}

//===========================================
// CreditsMenu::addToWorld
//===========================================
void CreditsMenu::addToWorld() {
   Menu::addToWorld();

   m_txtAlpha = 0.f;
   for (uint_t i = 0; i < m_textEntities.size(); ++i) {
      Colour col = m_textEntities[i]->getFillColour();
      col.a = m_txtAlpha;

      m_textEntities[i]->setFillColour(col);
   }
}

//===========================================
// CreditsMenu::removeFromWorld
//===========================================
void CreditsMenu::removeFromWorld() {
   Menu::removeFromWorld();
}

#ifdef DEBUG
//===========================================
// CreditsMenu::dbg_print
//===========================================
void CreditsMenu::dbg_print(ostream& out, int tab) const {
   for (int i = 0; i < tab; ++i) out << "\t";

   out << "CreditsMenu\n";

   Item::dbg_print(out, tab + 1);
   Menu::dbg_print(out, tab + 1);
}
#endif

//===========================================
// CreditsMenu::update
//===========================================
void CreditsMenu::update() {
   float32_t fr = gGetTargetFrameRate();

   Menu::update();

   if (m_txtAlpha < 1.f) { 
      m_txtAlpha += 1.f / (fr * m_fadeInTime);

      for (uint_t i = 0; i < m_textEntities.size(); ++i) {
         Colour col = m_textEntities[i]->getFillColour();
         col.a = m_txtAlpha;

         m_textEntities[i]->setFillColour(col);
      }
   }
}

//===========================================
// CreditsMenu::draw
//===========================================
void CreditsMenu::draw() const {
   Menu::draw();
}

//===========================================
// CreditsMenu::assignData
//===========================================
void CreditsMenu::assignData(const XmlNode data) {
   try {
      XML_NODE_CHECK(data, CreditsMenu)

      XmlNode node = data.firstChild();
      if (!node.isNull() && node.name() == "Menu") {
         Menu::assignData(node);
      }
   }
   catch (XmlException& e) {
      e.prepend("Error parsing XML for instance of class CreditsMenu; ");
      throw;
   }
}

//===========================================
// CreditsMenu::~CreditsMenu
//===========================================
CreditsMenu::~CreditsMenu() {}
