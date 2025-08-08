#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/motion/MotionController.h"
#include "task.h"

#include <map>
#include <initializer_list>
#include <vector>
#include <memory>

namespace Pinetime {
  namespace Applications {

    namespace Screens {

      class Magic8ball : public Screen {
      public:
        Magic8ball(Controllers::MotionController& motionController);
        ~Magic8ball() override;

        /// Open response type menu on long click
        bool OnTouchEvent(TouchEvents event) override;

        /// Function used for button callbacks
        void UpdateSelected(lv_obj_t* object, lv_event_t event);

        /// Closes menu if it's open
        bool OnButtonPushed() override;

        /// Checks accelerometer value and updates screen if needed
        void Refresh() override;

      private:
        void OpenMenu();
        void CloseMenu();

        /// Set state and whatever else is needed for the watch to start the refresh cycle. Does nothing if already fading in.
        void Start8BallRefresh();

        /// Simply choose a random string from the currently selected pool and return it
        [[nodiscard]] const char* getRandomString() const;

        Controllers::MotionController& motionController;
        lv_obj_t* mainText;
        // Describes the outline of the big triangle. Is an equilateral triangle with edge length 204 at the center of the screen.
        // Note that these points are used to draw a 16px thick line, so the actual boundary is 8px out from this
        // Main text is placed 5/8 way up on the triangle for better spacing.
        static constexpr lv_point_t triangleOutlinePoints[4] = {{18, 32}, {222, 32}, {120, 200}, {18, 32}};
        lv_obj_t* bigBlueTriangleOutline;
        lv_style_t bigBlueTriangleOutlineStyle;
        lv_obj_t* bigBlueTriangleFill;
        lv_style_t bigBlueTriangleFillStyle;
        lv_obj_t* btnPoolNext;
        lv_obj_t* btnPoolPrev;
        lv_obj_t* btnCloseMenu;
        lv_obj_t* currentPoolText;
        lv_obj_t* currentPoolBG;
        bool isMenuOpen;
        int activePool;
        lv_task_t* taskRefresh;
        uint32_t fadeStartTimestamp;
        lv_color_t lastTriangleFadeColor;
        uint32_t lastClickTimestamp;

        enum { Displaying, FadingIn } triangleDisplayState;

        /// Text shown on starting the application
        constexpr static char helpText[] = "Ask your quest-\nion and shake\nthe watch\nto get\nan ans-\nwer";

        /// Represents an entry in categoryContents. Each value is a unique category.
        enum CategoryType {
          None = 0,       // Dummy category containing a single error string as an option
          Positive,       // Original positive answers
          Deferring,      // Original deferring answers (like "Ask again")
          Negative,       // Original negative answers
          PositiveExtra,  // Custom positive answers
          DeferringExtra, // Custom deferring/silly answers (like "Does not compute")
          NegativeExtra,  // Custom negative answers
          Unknown,        // Non-answers and silliness
          Pawsitive,      // Cat themed positive answers
          Defurring,      // Cat themed deferring answers
          Mewgative,      // Cat themed negative answers
        };

        /// Entries in Pool objects
        /// @param weight The weight in the pool to give this item
        /// @param categoryType The category this member is referring to
        struct PoolMember {
          uint16_t weight;
          CategoryType categoryType;
        };

        /// The topmost structure in this app. Defines each option available in the app
        class Pool {
        public:
          /// @param name The name of the category to display
          /// @param usedCategories An initializer list of PoolMembers as the pool contents
          Pool(const char* name, const std::initializer_list<PoolMember> usedCategories)
            : name(name),
              categoryCount(usedCategories.size()) {
            categories = std::shared_ptr<PoolMember>(new PoolMember[categoryCount]);
            std::copy(usedCategories.begin(), usedCategories.end(), categories.get());
          }

          PoolMember& operator[](const size_t index) const {
            return categories.get()[index];
          }

          [[nodiscard]] size_t size() const {
            return categoryCount;
          }

          [[nodiscard]] const char* GetName() const {
            return name;
          }

        private:
          const char* name;
          size_t categoryCount;
          std::shared_ptr<PoolMember> categories;
        };

        // categoryContents and answerPools are not static because I don't want them in memory when not in use and there
        // should only ever be at most a single instance of this class anyway.

        // Comfortable text sizing guides:
        // 6 rows:
        // 123456789ABCDEF\n123456789ABCD\n123456789AB\n123456789\n1234567\n1234
        // 5 rows:
        // 123456789ABCDE\n123456789ABC\n123456789A\n12345678\n12345
        // 4 rows:
        // 123456789ABCD\n123456789AB\n123456789\n1234567
        // 3 rows:
        // 123456789ABC\n123456789A\n12345678
        // 2 rows:
        // 123456789AB\n123456789
        // 1 row:
        // 123456789A

        /// A map from CategoryType to a vector<char*> containing the results in each category.
        /// Must map every option in enum CategoryType (or all used ones)!
        const std::map<CategoryType, std::vector<const char*>> categories = {
          {None,
           {
             "Bug hap-\npened, plz\nreport",
           }},
          // Original answers from https://magic-8ball.com/magic-8-ball-answers/
          {Positive,
           {
             "It is\ncertain",
             "Without a\ndoubt",
             "It is deci-\ndedly so",
             "Yes defi-\nnitely",
             "You may\nrely on\nit",
             "As I see\nit, yes",
             "Most\nlikely",
             "Outlook\ngood",
             "Yes",
             "Signs point\nto yes",
           }},
          {Deferring,
           {
             "Reply hazy,\ntry again",
             "Ask again\nlater",
             "Better not\ntell you\nnow",
             "Cannot pre-\ndict now",
             "Concentrate\nand ask\nagain",
           }},
          {Negative,
           {
             "Don't count\non it",
             "My reply\nis no",
             "My sources\nsay no",
             "Outlook\nnot so\ngood",
             "Very\ndoubtful",
           }},
          {PositiveExtra,
           {
             "The Wall\nPeople are\nconfi-\ndent",
             "Why not?",
             "Horse of abso-\nlute agree-\nment and\nunderst-\nand-\ning:",
             "oo ye",
             // "If that's\nwhat you\nwant",
             "Eh,\nprobably",
             "Hell yeah",
             "Yes, but be\ncareful",
             "Yes, but\nwash your\nhands\nafter",
             "Not bad\nodds",
             "It will end\nfavorably",
             "Chances\ncould be\nworse",
             "Seems\npossible\nenough",
             "With enough\neffort,\nyes",
             ":)",
             "I see no\nissues\nwith it",
             "Disappoint-\ningly,\nyes",
             "Most vigor-\nously",
             "Probably",
             "Yes, and I\nwant to\nwatch",
             "No\n-Actually,\nyes",
             "Only after\n5pm",
             "Oh sweet\nheavens\nyes",
             "I think so",
             "Outlook\ngood, if\na little\nillegal",
             "Yes...?",
             "If the\nprophecy is\nto be be-\nlieved",
             "As surely\nas 2+2=4",
             "Yes!",
             "Yes!!!!",
             "Yes, and the\nhorse you\nrode in\non",
             "Obviously",
             "You'll\nprobably\nbe okay",
           }},
          {DeferringExtra,
           {
             // "The Wall\nPeople are-\nn't here\nnow",
             "Ask a\ntherapist\ninstead",
             "Error 500\n---\nInternal\nserver\nerror",
             "You just\nlost the\ngame, try\nagain",
             "RTFM and\nask again",
             "Try tea\nreading\ninstead",
             "Your quest-\nion is imp-\nortant to\nus",
             "Again~",
             "NOT\nANSWERING\nTHAT",
             "Apply glue\nliberally\nand ask\nagain",
             "Reevaluate\nyourself\nand ask\nagain",
             "Does not\ncompute",
             "Out for\nlunch",
             "BRB",
           }},
          {NegativeExtra,
           {
             "The Wall\nPeople are\nnot con-\nfident",
             "Eeny\nmeeny\nminey\nno",
             "Please re-\nconsider",
             "Strongly\nconsider an\naltern-\native",
             "Hold\n[shift]\nto run",
             "For legal\nreasons,\nno",
             "Nah",
             "Grandma\nwould be\nsad",
             // "Only with\n911 on\nspeed\ndial",
             "Just move\non",
             "Cease the\nthought",
             "Chances\nare poor",
             "Your confi-\ndence is\nmis-\nplaced",
             "Could be\nbetter",
             "NO NO NO NO\nNO NO NO\nNO NO",
             "Pray first",
             "Yes\n-Actually,\nno",
             "No, and nip-\nples don't\ngrow\nback",
             "Good\nluck...",
             "Lower your\nexpectat-\nions",
             "Answer is\nnot favor-\nable",
             "God no",
             "Not really",
             "No, that's\nnot legal\nanymore",
             "As surely\nas 2+2=5",
             "Sorry",
             "That's ill-\negal in 30\nstates",
             "Fffffor\nyou, no",
             "I'd bet\nagainst\nit",
             "Do you want\nflattery\nor hon-\nnesty?",
             "How about\nno",
           }},
          {Unknown,
           {
             "Answer is\nunclear",
             "What?",
             "Hard to\ntell",
             "I dunno",
             "Stop\nshaking\nme!",
             "Don't ask\nthat",
             "How would\nI know!",
             "Google it",
             "Bother\nsomeone\nelse",
             "But nobody\ncame",
             "Apple\njuice\nissues",
             "Is your\nfridge\nrunning?",
             "Is that\nHatsune\nMiku?",
             "Ew",
             "explosive\ndiarrhea",
             "Wario app-\nroaches",
             "so real",
             "Go back\nto bed",
             "Among us?",
             "Allan\nplease add\ndetails",
             "I knew you\nwould ask\nthat",
             "Your fly\nis down",
             "Bruh",
             "Oo I know\nthis one!",
             "What do you\nthink?",
             "Answer DLC\nonly\n$3.99!",
             "Do not\nthe cat",
             "I know what\nyou did",
             "A pickle\nwould\nsuffice",
             "CHICKEN\nJOCKEYYY",
             "Bring a\ncamera",
             "Bees?\nBees",
             "Take a\nbrick\nwith you",
             "Would your\nmother be\nproud?",
             "Remove the\nbones\nfirst",
             "Would you\nrather\ndrink hot\nsauce",
             "Dream big\nFart loud"
             "Gone\nfishing",
             "Shh I'm\nhiding",
             "Suddenly,\nhiccups",
             "Stop\neating\nworms",
           }},
          // The cat based ones were written at 5am. They were not meant to be good. Still, somehow I feel the need to apologize.
          // I would like to thank my brother for subjecting himself to reading articles of awful cat puns to use as inspiration.
          {Pawsitive,
           {
             "Of course,\nnya~",
             "Pawsitively\nso",
             "Outlook\npurrfect",
             "You have the\nright cati-\ntude for\nit",
             "Furry good\noutlook",
             "Yes, but\nafter a\nnap",
             "Don't be\nfurantic,\nodds are\ngood",
             "Nyo way for\nit to not\nbe the\ncase",
             "Mew've got\nthis",
             "Nyabso-\nlutely",
             "Nyes",
             "Your future\nis\npurright",
             "Affur-\nmative!",
           }},
          {Mewgative,
           {
             "A little\nbirdy told\nme no",
             "Nyo",
             "A cat-\nastrophe\napproach-\nes",
             "Are you feline\nlucky?\nYou shoul-\ndn't\nbe",
             "Nyanything\nbut that",
             "Your claws\nare sharp\nbut your\nhead is\ndull",
             "Fluff no!",
             "Nyanfor-\ntunately\nnyat",
             "Impawsible",
             "Oh,\npaw-lease",
             "Meowtlook\nnot good",
           }},
          {Defurring,
           {
             "Anything is\npawsible",
             "I'm just a\ncat, what\ndo you\nwant",
             "Meow is not\nthe time",
             "Food first",
             "Ask again,\nnya",
             "Answer is\nnyanclear",
             "Meow",
           }},
        };

        /// Vector of Pool objects containing information on what to display in the app.
        /// On app startup, index 0 is selected.
        std::vector<Pool> answerPools{
          {"Default",
           {
             {10, Positive},
             {5, Negative},
             {5, Deferring},
             {20, PositiveExtra},
             {10, NegativeExtra},
             {10, DeferringExtra},
             {1, Unknown}, // ~1.6% chance for a silly nonsense answer
           }},
          {"OG",
           {
             {10, Positive},
             {5, Negative},
             {5, Deferring},
           }},
          {"Fair",
           {
             {10, Positive},
             {5, Negative},
             {10, Deferring},
             {20, PositiveExtra},
             {10, NegativeExtra},
             {20, DeferringExtra},
             {1, Unknown}, // ~1.3% chance for a silly nonsense answer
           }},
          {"Rig - Yes",
           {
             {15, Positive},
             {5, Deferring},
             {30, PositiveExtra},
             {10, DeferringExtra},
           }},
          {"Rig - No",
           {
             {15, Negative},
             {5, Deferring},
             {30, NegativeExtra},
             {10, DeferringExtra},
           }},
          {"Cat",
           {
             {10, Pawsitive},
             {5, Mewgative},
             {5, Defurring},
           }},
        };
      };
    }

    template <>
    struct AppTraits<Apps::Magic8ball> {
      static constexpr Apps app = Apps::Magic8ball;
      static constexpr const char* icon = "8";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Magic8ball(controllers.motionController);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}