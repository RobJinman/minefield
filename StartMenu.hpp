#ifndef __START_MENU_HPP__
#define __START_MENU_HPP__


#include <boost/shared_ptr.hpp>
#include <dodge/EEvent.hpp>
#include <dodge/Sprite.hpp>
#include <dodge/WinIO.hpp>
#include "dodge/xml/xml.hpp"
#include "Item.hpp"
#include "MenuItem.hpp"


class StartMenu : public Item, public Dodge::Sprite {
   public:
      StartMenu(const Dodge::XmlNode data);
      StartMenu(const StartMenu& copy);
      StartMenu(const StartMenu& copy, long name);

      inline void setActive(bool b);

      virtual void draw() const;
      virtual void update();

      virtual size_t getSize() const;
      virtual StartMenu* clone() const;
      virtual void addToWorld();
      virtual void removeFromWorld();

      virtual void onEvent(const Dodge::EEvent* event);
      virtual void assignData(Dodge::XmlNode data);
#ifdef DEBUG
      virtual void dbg_print(std::ostream& out, int tab = 0) const;
#endif
      virtual ~StartMenu();

   private:
      void init();
      void keyDown(int key);
      void menuItemClick(Dodge::pEntity_t entity);

      Dodge::WinIO m_win;
      bool m_active;
      std::vector<pMenuItem_t> m_menuItems;
};

typedef boost::shared_ptr<StartMenu> pStartMenu_t;

//===========================================
// StartMenu::setActive
//===========================================
inline void StartMenu::setActive(bool b) {
   m_active = b;
}


#endif