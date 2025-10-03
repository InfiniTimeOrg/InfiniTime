#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/LittleVgl.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"
#include "components/motor/MotorController.h"

#include <limits>
#include <memory>
#include <vector>
#include <cmath>
#include <any>
#include <numbers>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      /// Simple struct to contain both a color and its opacity. NOT premultiplied.
      struct ColorWithOpacity {
        lv_color_t color;
        lv_opa_t alpha;
      };


      /// Describes what type of noise to use for a TextureLayer.
      enum class LayerNoise {
        Blank = 0,   // Solid color
        Simple,      // Random value per pixel
        Perlin,      // Perlin noise. Relatively expensive.
        ShapeSquare, // Repeating square pattern
        // Repeating triangles pointing up. Remember bounding box is square unless set otherwise.
        // For equilateral triangles, use scaleX = scaleY * tan(30deg) * 2
        // Noise value of 1 means it fills in the entire square bounding box. Set gradient max to 0.5 to get perfect repeating triangles.
        ShapeTriangle,
        // Repeating circle pattern. Remember bounding box is square unless set otherwise.
        // Noise value of 1 means it fills in the entire square bounding box. Set gradient max to 1/sqrt(2) to get max size circles.
        ShapeCircle,
      };

      /// Used in TextureLayerData[type] objects. Describes a simple, 2 point gradient. The gradients this describes are linearly
      /// interpolated between gradientStart and gradientEnd.
      class GradientData {
      public:
        /// Construct by using doubles for gradientStart and gradientEnd. Simply a range between [0,1].
        /// @param gradientStart Defines the start of the linear gradient portion. Must be in range [0.0, 1.0].
        /// @param gradientEnd Same as gradientStart but for the gradient end value. Must be > gradientStart.
        /// @param fromColor The color to use for the low end of the gradient.
        /// @param fromAlpha The alpha for fromColor.
        /// @param toColor The color to use for the high end of the gradient.
        /// @param toAlpha The alpha for toColor.
        GradientData(float gradientStart,
                     float gradientEnd,
                     lv_color_t fromColor,
                     lv_opa_t fromAlpha,
                     lv_color_t toColor,
                     lv_opa_t toAlpha);

        /// Construct by using doubles for gradientStart and gradientEnd. Simply a range between [0,1].
        /// @param gradientStart Defines the start of the linear gradient portion. Must be in range [0.0, 1.0].
        /// @param gradientEnd Same as gradientStart but for the gradient end value. Must be > gradientStart.
        /// @param color The color to use for the entire gradient.
        /// @param alpha The alpha to use for the entire gradient.
        GradientData(float gradientStart,
                     float gradientEnd,
                     lv_color_t color,
                     lv_opa_t alpha);

        /// Interpolate the gradient based on the given position. Returns black at 0 opacity if clipped off by clipLow or clipHigh.
        [[nodiscard]] ColorWithOpacity Interpolate(uint16_t chosenValue) const;

        /// Chainable function which sets if the gradient should clip above and below the provided endpoints.
        /// @param clipLow If true, if generated value is < gradientStart then the pixel is unmodified instead of taking fromColor.
        /// @param clipHigh Same as clipLow for values > gradientEnd.
        /// @return The object it was called with.
        GradientData& SetClip(bool clipLow, bool clipHigh);

        /// Chainable function which sets the end point locations of the gradient.
        /// @param start Defines the start of the linear gradient portion. Must be in range [0.0, 1.0].
        /// @param end Same as gradientStart but for the gradient end value. Must be > gradientStart.
        /// @return The object it was called with.
        GradientData& SetEndpoints(float start, float end);

        /// Chainable function which sets the end point colors of the gradient.
        /// @param start The color to use for the low end of the gradient.
        /// @param end The color to use for the high end of the gradient.
        /// @return The object it was called with.
        GradientData& SetColors(lv_color_t start, lv_color_t end);

        /// Chainable function which sets both end point colors to the same value.
        /// @param color The color to use for both ends of the gradient.
        /// @return The object it was called with.
        GradientData& SetColor(lv_color_t color);

        /// Chainable function which sets the end point alphas of the gradient.
        /// @param start The alpha for fromColor.
        /// @param end The alpha for toColor.
        /// @return The object it was called with.
        GradientData& SetAlphas(lv_opa_t start, lv_opa_t end);

        /// Chainable function which sets both end point alphas to the same value.
        /// @param alpha The alpha for fromColor.
        /// @return The object it was called with.
        GradientData& SetAlpha(lv_opa_t alpha);

      private:
        uint16_t gradientStart;
        uint16_t gradientEnd;
        lv_color_t fromColor;
        lv_opa_t fromAlpha;
        lv_color_t toColor;
        lv_opa_t toAlpha;
        bool clipLow = false;
        bool clipHigh = false;
      };


      /// Struct containing data for Blank type noise
      /// @param color The color to fill this layer with.
      /// @param alpha The alpha value for the color to fill the layer with.
      struct TextureLayerDataBlank {
        lv_color_t color;
        lv_opa_t alpha;
      };


      /// Struct containing data for Simple type noise. Needs no additional information aside from the gradient.
      /// @param gradientData Contains information for the gradient to use.
      struct TextureLayerDataSimple {
        GradientData gradientData;
      };


      // type T must be a child type of TextureLayerDataBaseShiftable (I couldn't figure out how to check it in code)
      template <typename T>
      struct TextureLayerDataBaseMovable {
        /// @param gradientData Contains information about the gradient to use.
        /// @param scale The scale of the noise in pixels. Must be >0.
        TextureLayerDataBaseMovable(const GradientData& gradientData,
                                    uint16_t scale);

        /// Chainable function to set the offsets for this noise.
        /// For any scales that do not divide evenly into 65536, the texture will have a seam between pixels -32768 and 32767.
        /// To keep it offscreen, keep shifts in range of [-32768 + SCREEN_AXIS_SIZE, 32767].
        /// This function automatically moves the given shifts if they would display the seam!
        /// @param x Move the noise right by this many pixels
        /// @param y Move the noise down by this many pixels
        /// @return The object it was called with
        T& SetShift(int16_t x, int16_t y);

        /// Chainable function to set per-axis scale for this noise.
        /// @param x Set the horzontal scale to this, in pixels
        /// @param y Set the verticalscale to this, in pixels
        /// @return The object it was called with
        T& SetScale(uint16_t x, uint16_t y);

        /// Chainable function to rescale the noise.
        /// @param scale Set the scale to this on both axes
        /// @return The object it was called with
        T& SetScale(uint16_t scale);

        GradientData gradientData;
        uint16_t scaleX;
        uint16_t scaleY;
        int16_t shiftX;
        int16_t shiftY;
      };


      /// Struct containing data for Perlin type noise.
      struct TextureLayerDataPerlin : TextureLayerDataBaseMovable<TextureLayerDataPerlin> {
        using TextureLayerDataBaseMovable::TextureLayerDataBaseMovable;
      };


      /// Struct containing data for ShapeSquare type noise.
      struct TextureLayerDataSquare : TextureLayerDataBaseMovable<TextureLayerDataSquare> {
        using TextureLayerDataBaseMovable::TextureLayerDataBaseMovable;
      };


      /// Struct containing data for ShapeTriangle type noise.
      struct TextureLayerDataTriangle : TextureLayerDataBaseMovable<TextureLayerDataTriangle> {
        using TextureLayerDataBaseMovable::TextureLayerDataBaseMovable;
      };


      /// Struct containing data for ShapeCircle type noise.
      struct TextureLayerDataCircle : TextureLayerDataBaseMovable<TextureLayerDataCircle> {
        using TextureLayerDataBaseMovable::TextureLayerDataBaseMovable;
      };


      /// A layer to be used in TextureGenerator. Is more of a means of providing a single unified interface for all types of layers.
      class TextureLayer {
      public:
        /// @param noiseType The type of noise to use. See NoiseType for descriptions.
        /// @param textureLayerData The data to pass to the noise generator function. Must be one of the TextureLayerData[Type] structs.
        TextureLayer(LayerNoise noiseType, const std::any& textureLayerData)
          : noiseType{noiseType},
            textureLayerData{textureLayerData} {
        }

        /// Calculate an entire layer and merge it into the provided buffer. Prefer using this over CalculatePixel directly.
        /// Buffer gets populated starting at index 0, but with the part of the texture described by the coordinates.
        /// @param buffer A buffer of already populated pixels which this layer gets overlaid onto.
        /// @param x1 Left side of the calculated bounds. Inclusive.
        /// @param y1 Top side of the calculated bounds. Inclusive.
        /// @param x2 Right side of the calculated bounds. Inclusive.
        /// @param y2 Bottom side of the calculated bounds. Inclusive.
        /// @return Nothing, but buffer gets updated in-place.
        void CalculateLayer(lv_color_t* buffer, lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2) const;

        /// Calculate the value of the target pixel for this layer.
        /// @param x The x-coordinate of the pixel to calculate. Layers are infinite, so this can be any value.
        /// @param y The y-coordinate of the pixel to calculate. Layers are infinite, so this can be any value.
        /// @return The color and opacity of the pixel.
        ColorWithOpacity CalculatePixel(lv_coord_t x, lv_coord_t y) const;

        /// Chainable function to set the drawn bounds for this layer.
        /// Coordinates are screenspace and do not account for shifts in the underlying texture layers..
        /// @param minX The minimum horizontal texture pixel to still draw.
        /// @param maxX The maximum horizontal texture pixel to still draw.
        /// @param minY The minimum vertical texture pixel to still draw.
        /// @param maxY The maximum vertical texture pixel to still draw.
        /// @return The object it was called with
        TextureLayer& SetBounds(int16_t minX, int16_t maxX, int16_t minY, int16_t maxY);

      private:
        /// Handles Blank noise type layers.
        inline ColorWithOpacity CalculateBlankPixel() const;
        /// Handles Simple noise type layers.
        inline ColorWithOpacity CalculateSimplePixel() const;
        /// Handles Perlin noise type layers.
        inline ColorWithOpacity CalculatePerlinPixel(lv_coord_t x, lv_coord_t y) const;
        /// Handles ShapeSquare noise type layers.
        inline ColorWithOpacity CalculateSquarePixel(lv_coord_t x, lv_coord_t y) const;
        /// Handles ShapeTriangle noise type layers.
        inline ColorWithOpacity CalculateTrianglePixel(lv_coord_t x, lv_coord_t y) const;
        /// Handles ShapeCircle noise type layers.
        inline ColorWithOpacity CalculateCirclePixel(lv_coord_t x, lv_coord_t y) const;

        LayerNoise noiseType;
        std::any textureLayerData;
        int16_t minXBound = std::numeric_limits<int16_t>::min();
        int16_t maxXBound = std::numeric_limits<int16_t>::max();
        int16_t minYBound = std::numeric_limits<int16_t>::min();
        int16_t maxYBound = std::numeric_limits<int16_t>::max();
      };


      /// The overall generator for textures.
      class TextureGenerator {
      public:
        TextureGenerator() = default;

        /// Add a noise layer. Makes a copy so the layer can be modified and reused afterward.
        void AddTextureLayer(const TextureLayer& layer) {
          layers.push_back(layer);
        }

        /// Populates the provided buffer with the specified portion of the screen.
        /// Buffer gets populated starting at index 0, but with the part of the texture described by the coordinates.
        /// @param buffer A buffer of already populated pixels which this layer gets overlaid onto.
        /// @param x1 Left side of the calculated bounds. Inclusive.
        /// @param y1 Top side of the calculated bounds. Inclusive.
        /// @param x2 Right side of the calculated bounds. Inclusive.
        /// @param y2 Bottom side of the calculated bounds. Inclusive.
        /// @return Nothing, but buffer gets updated in-place.
        void GetBlock(lv_color_t* buffer, lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2);

        /// Calculate a single pixel of the texture. Prefer GetBlock() over calling this directly.
        /// @param x The x-coordinate of the pixel to calculate.
        /// @param y The y-coordinate of the pixel to calculate.
        /// @return The color of the pixel.
        lv_color_t GetPixel(lv_coord_t x, lv_coord_t y) const;

      private:
        std::vector<TextureLayer> layers{};
      };


      /// You like eating moss, don't you? This can tell exactly how much you do.
      /// This is really just a specialized 2D bit array.
      class MossMunchTracker {
      public:
        /// Create a MossMunchTracker with the provided screen size
        MossMunchTracker(uint16_t width, uint16_t height);

        MossMunchTracker(MossMunchTracker& other);
        ~MossMunchTracker();
        MossMunchTracker& operator=(MossMunchTracker const& other);

        /// Resets the tracker to be full of moss again.
        void Reset();

        /// Makes a bite mark on the tracker canvas at the given center coordinates. Uses IsPointInBiteMark().
        /// @return True if any moss has been eaten with this call (for feedback purposes), else false.
        bool Munch(lv_coord_t centerX, lv_coord_t centerY);

        /// @return True if all the moss has been eaten, False if the user has more work to do
        [[nodiscard]] bool IsFinished() const;

      private:
        /// @return True if the bit was set before unsetting it (if ate the moss).
        inline bool MunchBit(lv_coord_t x, lv_coord_t y);

        uint8_t* grid;         // 2D bit array. 1 means there is still moss there, 0 means it's been eaten
        uint16_t gridByteSize; // The 1D length of the array. May not equal width * height / 8.
        uint16_t width;        // The width of the canvas
        uint16_t height;       // The height of the canvas
        uint32_t mossMunched;  // A count of how many pixels of moss have been eaten
      };


      /// Each type of paintable moss.
      enum class MossType : uint8_t {
        Error = 0,            // Magenta and black pattern for errors
        Basic,                // Simple green moss
        Blue,                 // Simple blue moss
        Digital,              // Blocky green moss
        Gray,                 // Simple gray-green moss
        Dark,                 // Dark gray moss
        DarkGlowyBlue,        // Dark gray moss with blue glowy bits
        DarkGlowyOrange,      // Dark gray moss with orange glowy bits
        Wild,                 // Simple yellowish moss
        SpanishMossquisition, // Nobody expects this one (hanging tan moss)
        Soul,                 // Hazy green-brown-purple souls hanging near the ground
        Fiery,                // Moss on fire
        RedGlow,              // Glowing red moss
        Bread,                // Tan flatbread
        Carpet,               // Green with shape patterns
        TVStatic,             // Pure B/W static
        Cultivated,           // Smooth green texture
        Mousse,               // Brown airy treat
        Sunrise,              // Simple orange tinted moss
        Suspicious,           // Green moss with red+yellow speckles
        TreeMoss,             // Green moss on brown bark
        MossMoss,             // Higher contrast moss
        QueerPride,           // Pride flag colored moss
        QueerTrans,           // Trans flag colored moss
        QueerLesbian,         // Lesbian flag colored moss
        QueerGay,             // Gay flag colored moss
        QueerBi,              // Bisexual flag colored moss
        QueerPan,             // Pansexual flag colored moss
        QueerEnby,            // Non-binary flag colored moss
        QueerAce,             // Asexual flag colored moss
        Grass,                // Grass strands
        Big,                  // Zoomed in moss
        Zombie,               // Simple dark green moss
        Moon,                 // Moon color with crescent shapes
        Aurora,               // Northern lights (don't ask)
        Sickened,             // Colorful moss (when getting sick)
        Lichen,               // Light patches of lichen on rock
        Glass,                // Light blue with shiny glassy reflections
        ShowerCurtain,        // Greenish, stretched vertically
        Brick,                // Reddish brick with grout
        Stars,                // Small white sparkles on dark gray background
        EntireHouse,          // Just a whole house
        GoodLooking,          // Moss abs
        Irradiated,           // glowing green
        Monster,              // dark moss + vague red splotches
        Frowny,               // just a frowny face
        Blanket,              // Green plaid blanket
        DarkBubbly,           // Circles of moss with little halos
        Mossta,               // Green strands of moss pasta on a plate
        Gold,                 // Super-rare golden moss
        Egg,                  // Randomly generated Easter egg
        AteAllMoss,           // Congratulations trophy
        // MAX AND MIN FOR DBGALLMOSS MODE, MUST NEVER BE USED ELSEWHERE
        Max = AteAllMoss, // MUST be set to last real moss
        Min = Error,      // MUST be set to first real moss
      };


      /// Manages the sequence of events displayed between mosses
      class MossStory {
      public:
        MossStory();

        /// Increment the internal story state and returns a StoryBeatDescriptor containing what needs to be displayed
        void IncrementStory();

        /// Return the current text for the active story beat
        [[nodiscard]] const char* GetCurrentStoryText() const {
          return curStoryText.c_str();
        }

        /// Return the current moss type for the active story beat
        [[nodiscard]] MossType GetCurrentMossType() const {
          return curMossType;
        }

        /// Return the current scene text for the active scene. Uses color, so have the LVGL label allow recoloring.
        [[nodiscard]] const char* GetCurrentSceneText() const;

        /// For debugging / showing off purposes.
        /// Does NOT update anything other than the current scene.
        void DebugIncrementScene();

      private:
        /// Contains every single possible unique story beat. Includes scenes which have the same kind of moss but different text.
        /// Scenes may also result in multiple types of mosses or texts.
        enum class StoryBeat : uint8_t {
          Error = 0,
          // MOSSES THAT CAN BE FOUND IN FOREST AND CIVILIZATION
          Nighttime,     // Random event
          NighttimeSky,  // Might happen after nighttime
          Sunrise,       // After nighttime
          SusMoss,       // Speckled moss
          SusMossResult, // It didn't really look edible anyway
          TreeMoss,      // Moss on tree
          Grass,         // Couldn't find moss :(
          Supportive,    // Pride flags
          Fiery,         // Moss on fire
          MossHell,      // Portal to Moss Hell opens
          SimpleMoss,    // Ordinary
          // HOUSE EXCLUSIVE MOSSES
          HouseExitCiv,       // Exit from House to Civilization
          HouseShowerCurtain, // Eat their shower curtain
          HouseDigitalMoss,   // Eat their router or something
          HouseTVStatic,      // Eat their TV!!
          HouseCarpet,        // Carpet of moss. Probably.
          HouseMystery,       // The unknown
          HouseAurora,        // An unforgettable luncheon happened here
          HouseZombie,        // Zombies in the basement
          HouseBlanket,       // Blanket of moss. Probably.
          // FOREST EXCLUSIVE MOSSES
          ForestIntro,    // Shown at the beginning of runs
          ForestIntroPT2, // Shown after the beginning of runs
          ForestWild,     // Standard wild moss
          ForestBig,      // XL moss
          ForestMossMoss, // Moss growing on moss
          Crypt,          // Status update!
          ForestExitCiv,  // Exit from Forest to Civilization
          ForestExitCave, // Exit from Forest to Cave
          ForestStars,    // A mystery.
          // CIVILIZATION EXCLUSIVE MOSSES
          CivPool,       // Fall in a pool.
          CivBread,      // Bread? Moss? Idk.
          CivCultivated, // Someone's been cultivating moss
          CivMousse,     // Close enough
          CivExitForest, // Exit from Civilization to Forest
          CivExitHouse,  // Exit from Civilization to House
          CivIrradiated, // I'm wakin' up, to ash and dust...
          CivMossta,     // Disgusting.
          // CAVE EXCLUSIVE MOSSES
          CaveLichen,      // Can't find moss, eat lichen instead
          CaveDarkMoss,    // Dark moss
          CaveDimLitMoss,  // Dimly lit moss
          CaveGlowingMoss, // Glowing moss!
          CaveZombie,      // Zombie moss
          CaveMonster,     // Share a meal
          CaveExitForest,  // Exit from Cave to Forest
          CaveBubbly,      // Bubbly moss...?
          // MOSSES THAT CAN BE FOUND ANYWHERE
          SpanishMossquisition, // Nobody expects this one.
          GoodLooking,          // Please don't ask.
          GoldenMoss,           // Rare, and tasty!
          LostTheGame,          // :(
          // ONLY USED IN DBGALLMOSS SCENE
          CycleMoss, // Cycles shown moss
          // ONLY USED IN EGG SCENE
          Egg, // Easter egg
        };

        // Where the player is currently (affects available mosses)
        enum class StoryScene : uint8_t {
          Error = 0,
          Forest,       // What it says on the tin. Where you first spawn
          Cave,         // In Forest, found a cave
          Civilization, // Nondescript city
          House,        // In Civilization, ate your way into someone's house
          DbgAllMoss,   // Simply cycle through all mosses
          Egg,          // Easter egg
          // USED WHEN CYCLING THROUGH SCENES, MUST NEVER BE USED ELSEWHERE
          Max = Egg,   // MUST be set to last real scene
          Min = Error, // MUST be set to first real scene
        } currentScene;

        /// Checks if a StoryBeat is in recent events
        /// @param storyBeat What to check for
        /// @param howRecent How many recent beats to check. Values above 5 do nothing, since that's the size of the history.
        [[nodiscard]] bool IsInRecentBeats(StoryBeat storyBeat, uint16_t howRecent = 5) const;
        /// Checks how many times a StoryScene happened recently
        /// @param storyScene What to check for
        /// @param howRecent How many recent scenes to check. Values above 4 do nothing, since that's the size of the history.
        [[nodiscard]] uint16_t CountInRecentScenes(StoryScene storyScene, uint16_t howRecent = 4) const;

        /// If the provided event has not happened recently, push it onto the vector
        /// @param possibleNextBeats The vector to push onto
        /// @param storyBeat The story beat to check and push
        /// @param howRecent If the story beat happened within this many previous events, don't push it on
        void PushIfNotHappenedRecently(std::vector<StoryBeat>& possibleNextBeats, StoryBeat storyBeat, uint16_t howRecent = 5) const;

        /// Simple function to test a chance. Uses std::rand().
        /// @param chanceTrue Chance of returning true. Must be in range [0,100].
        [[nodiscard]] inline static bool PercentChance(uint8_t chanceTrue);

        /// Based on current state, get a valid next StoryBeat. Does NOT increment state.
        /// This is the main function to call which delegates control to more specialized functions.
        [[nodiscard]] StoryBeat GetNextStoryBeat();

        /// Get the current story beat, knowing that the scene is in Forest
        [[nodiscard]] StoryBeat GetNextStoryBeatForest() const;
        /// Get the current story beat, knowing that the scene is in Civilization
        [[nodiscard]] StoryBeat GetNextStoryBeatCivilization() const;
        /// Get the current story beat, knowing that the scene is in Cave
        [[nodiscard]] StoryBeat GetNextStoryBeatCave() const;
        /// Get the current story beat, knowing that the scene is in House
        [[nodiscard]] StoryBeat GetNextStoryBeatHouse() const;
        /// Always returns CycleMoss. Only accessible through debug shortcuts
        [[nodiscard]] static StoryBeat GetNextStoryBeatAllMoss();
        /// Always returns CycleMoss. Only accessible through debug shortcuts
        [[nodiscard]] static StoryBeat GetNextStoryBeatEgg();
        /// Takes a vector and appends all possible next beats, or returns the beat if it must be the next beat.
        /// @return StoryBeat::Error if simply added all needed beats to the pool, or a valid StoryBeat if one was already chosen.
        StoryBeat PushSharedCivForestBeats(std::vector<StoryBeat>& possibleNextBeats) const;
        /// Takes a vector and appends all possible next beats, or returns the beat if it must be the next beat.
        /// @return StoryBeat::Error if simply added all needed beats to the pool, or a valid StoryBeat if one was already chosen.
        StoryBeat PushSharedBeats(std::vector<StoryBeat>& possibleNextBeats) const;

        /// Updates the internal text, moss type, scene, and histories with the given StoryBeat.
        /// Does NOT increment mossEaten or timeInCurrentScene, but may set timeInCurrentScene to 0 if a scene change occurs.
        void UpdateCurrentVars(StoryBeat storyBeat);

        StoryBeat recentBeats[5];    // History of recent StoryBeats. New items get added on the right side.
        StoryScene recentScenes[4];  // History of recent StoryScenes. New items get added to the right side.
        uint16_t mossEaten;          // Count of how many mosses the user has eaten.
        uint16_t timeInCurrentScene; // How much time has been spent in the current scene (first event after switching to scene is 1)

        std::string curStoryText; // Story text to display right now
        MossType curMossType;     // Moss type to display right now
      };


      /// Simulates moss. Not enough to capture its true grace, but enough to offer a glimpse.
      class MossSimulator : public Screen {
      public:
        MossSimulator(Components::LittleVgl& lvgl, Controllers::MotorController& motorController, System::SystemTask& systemTask);
        ~MossSimulator() override;

        /// Main touch input to app
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

        /// Only used for debug scene switching at program start
        bool OnButtonPushed() override;

        /// Discard all extra touch events such as swipes
        bool OnTouchEvent(TouchEvents) override {
          return true;
        }

        /// Function used for button callbacks
        // void UpdateSelected(lv_obj_t* object, lv_event_t event);

        void Refresh() override;

      private:
        /// Based on story progress, populates the texGen variable with a suitable texture generator
        void CreateTexGen();

        /// Handle main loop for growing state
        void RefreshGrowing();
        /// Handle main loop for eating state
        void RefreshEating();
        /// Handle main loop for story display state
        void RefreshStoryDisp();
        // DbgSceneSelect doesn't need any refresh function
        /// Handle debug scene change state
        void RefreshDbgEntry();
        /// Draw a bite mark at the given center coordinates
        void DrawBiteMark(lv_coord_t centerX, lv_coord_t centerY);
        /// Draw a single square of the growing animation
        void DrawGrowingTile();

        /// Perform tasks needed to switch to growing state
        void SwitchToGrowing();
        /// Perform tasks needed to switch to eating state
        void SwitchToEating();
        /// Perform tasks needed to switch to story display state. Does NOT set storyText or sceneText contents.
        /// Call after setting storyText and sceneText contents, since this aligns the object.
        void SwitchToStoryDisp();
        /// Perform tasks needed to switch to debug entry state
        /// This should
        void SwitchToDbgEntry();
        /// Perform tasks needed to switch to debug scene change state
        /// Should only come after DbgEntry state, but will work correctly if called out of order
        void SwitchToDbgSceneSelect();

        /// If stillHoldingTap is set, checks if user has since let go of tap. If so, unsets stillHoldingTap.
        void PollStillHoldingTap();

        void SwitchActiveBuffer() {
          activeBuffer = activeBuffer == buffer1 ? buffer2 : buffer1;
        }

        lv_color_t buffer1[100]; // Buffer for drawing stuff to screen
        lv_color_t buffer2[100]; // Alternate buffer to allow switching
        // Use SwitchActiveBuffer() each time before using the buffer to try and prevent timing issues
        lv_color_t* activeBuffer = buffer1;
        TextureGenerator texGen;           // Texture generator object
        uint16_t growingChunk;             // Index, from 0, of the current chunk for the Growing phase
        MossMunchTracker munchTracker;     // Tracker for how much moss has been eaten
        lv_obj_t* storyText;               // LVGL object to display story text during StoryDisp phase
        lv_obj_t* sceneText;               // LVGL object to display scene text during StoryDisp phase
        bool stillHoldingTap;              // If user is still holding tap since last check
        uint32_t stillHoldingTapStartTime; // When the stillHoldingTap check started
        uint16_t remainingTapHoldLeniency; // How many more refreshes until touch is considered released
        MossStory mossStory;               // Story controller object
        uint32_t lastEatingBuzzTime;       // Last time the watch buzzed because the user ate some moss

        // Various system objects
        Components::LittleVgl& lvgl;
        Controllers::MotorController& motorController;
        Pinetime::System::WakeLock wakeLock;
        lv_task_t* taskRefresh;

        /// The state of the screen
        enum { Growing, Eating, StoryDisp, DbgSceneSelect, DbgEntry } state;
      };
    }


    template <>
    struct AppTraits<Apps::MossSimulator> {
      static constexpr Apps app = Apps::MossSimulator;
      static constexpr const char* icon = Screens::Symbols::seedling;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::MossSimulator(controllers.lvgl,
                                          controllers.motorController,
                                          *controllers.systemTask);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}