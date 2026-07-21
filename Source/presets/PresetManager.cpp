#include "presets/PresetManager.h"
#include "assets/ResourcePaths.h"

void PresetManager::initialise (const juce::File& factoryDir, const juce::File& userDir)
{
    factoryDir_ = factoryDir;
    userDir_ = userDir;
    userDir_.createDirectory();
    loadFavorites();
    rescan();
}

void PresetManager::loadFavorites()
{
    favorites_.clear();
    auto f = userDir_.getChildFile ("favorites.json");
    if (! f.existsAsFile()) return;
    auto json = juce::JSON::parse (f.loadFileAsString());
    if (auto* arr = json.getArray())
        for (const auto& v : *arr)
            favorites_.addIfNotAlreadyThere (v.toString());
}

void PresetManager::saveFavorites() const
{
    juce::Array<juce::var> arr;
    for (const auto& n : favorites_)
        arr.add (n);
    userDir_.getChildFile ("favorites.json")
        .replaceWithText (juce::JSON::toString (juce::var (arr), true));
}

bool PresetManager::isFavorite (const juce::String& name) const
{
    return favorites_.contains (name);
}

void PresetManager::toggleFavorite (const juce::String& name)
{
    if (favorites_.contains (name))
        favorites_.removeString (name);
    else
        favorites_.add (name);
    saveFavorites();
}

juce::String PresetManager::collectionForCategory (const juce::String& category)
{
    if (category.containsIgnoreCase ("Bass") || category.containsIgnoreCase ("Moog"))
        return "Bass";
    if (category.containsIgnoreCase ("Pad") || category.containsIgnoreCase ("Atmos"))
        return "Pads";
    if (category.containsIgnoreCase ("Lead"))
        return "Leads";
    if (category.containsIgnoreCase ("Piano") || category.containsIgnoreCase ("Keys"))
        return "Keys";
    if (category.containsIgnoreCase ("Vocal") || category.containsIgnoreCase ("Choir"))
        return "Vocals";
    if (category.containsIgnoreCase ("Bell") || category.containsIgnoreCase ("Scape")
        || category.containsIgnoreCase ("Texture"))
        return "Textures";
    if (category.containsIgnoreCase ("Epic") || category.containsIgnoreCase ("Cinematic"))
        return "Cinematic";
    if (category.containsIgnoreCase ("Dark"))
        return "Experimental";
    if (category.containsIgnoreCase ("FX") || category.containsIgnoreCase ("Sample"))
        return "FX";
    return category.isNotEmpty() ? category : "Factory";
}

void PresetManager::scanAssetFiles()
{
    auto addAsset = [this] (const juce::File& file, LibraryItemKind kind, const juce::String& cat) {
        PresetInfo info;
        info.file = file;
        info.name = file.getFileNameWithoutExtension().replaceCharacter ('_', ' ');
        info.category = cat;
        info.author = "Scorion Factory";
        info.engine = kind == LibraryItemKind::Wavetable ? "Wavetable"
                    : kind == LibraryItemKind::Sample ? "Sampler" : "Virtual Analog";
        info.kind = kind;
        info.mood = kind == LibraryItemKind::Sample ? "organic" : "textural";
        info.energy = 0.4f;
        info.brightness = 0.5f;
        info.warmth = 0.45f;
        info.artworkSeed = info.name.hashCode();
        info.tags.add (kind == LibraryItemKind::Wavetable ? "wavetable" : "sample");
        info.tags.add (cat.toLowerCase());
        presets_.push_back (std::move (info));
    };

    auto wtDir = ResourcePaths::factoryWavetables();
    if (wtDir.isDirectory())
        for (auto& f : wtDir.findChildFiles (juce::File::findFiles, false, "*.wtbin"))
            addAsset (f, LibraryItemKind::Wavetable, "Textures");

    auto smDir = ResourcePaths::factorySamples();
    if (smDir.isDirectory())
        for (auto& f : smDir.findChildFiles (juce::File::findFiles, false, "*.wav"))
            addAsset (f, LibraryItemKind::Sample, "FX");
}

void PresetManager::rescan()
{
    presets_.clear();
    auto scan = [this] (const juce::File& dir) {
        if (! dir.isDirectory()) return;
        for (const auto& f : juce::RangedDirectoryIterator (dir, true, "*.json", juce::File::findFiles))
        {
            if (f.getFile().getFileName() == "favorites.json") continue;
            auto text = f.getFile().loadFileAsString();
            auto json = juce::JSON::parse (text);
            if (! json.isObject()) continue;
            PresetInfo info;
            info.file = f.getFile();
            info.name = json.getProperty ("name", f.getFile().getFileNameWithoutExtension()).toString();
            info.category = json.getProperty ("category", "User").toString();
            info.author = json.getProperty ("author", "Scorion").toString();
            info.engine = json.getProperty ("engine", "Virtual Analog").toString();
            info.mood = json.getProperty ("mood", "neutral").toString();
            info.energy = (float) json.getProperty ("energy", 0.5);
            info.brightness = (float) json.getProperty ("brightness", 0.5);
            info.warmth = (float) json.getProperty ("warmth", 0.5);
            info.artworkSeed = (int) json.getProperty ("artworkSeed", info.name.hashCode());
            info.kind = LibraryItemKind::Preset;
            if (auto* tags = json.getProperty ("tags", {}).getArray())
                for (const auto& t : *tags)
                    info.tags.add (t.toString());
            presets_.push_back (std::move (info));
        }
    };
    scan (factoryDir_);
    scan (userDir_);
    scanAssetFiles();

    if (presets_.empty())
    {
        PresetInfo init;
        init.name = "Init VA";
        init.category = "Leads";
        init.engine = "Virtual Analog";
        init.mood = "neutral";
        init.tags.add ("init");
        presets_.push_back (init);
    }
    currentIndex_ = juce::jlimit (0, (int) presets_.size() - 1, currentIndex_);
}

juce::String PresetManager::getCurrentName() const
{
    if (presets_.empty()) return "Init";
    return presets_[(size_t) currentIndex_].name;
}

juce::String PresetManager::getCurrentCategory() const
{
    if (presets_.empty()) return {};
    return presets_[(size_t) currentIndex_].category;
}

juce::var PresetManager::paramsToJson (juce::AudioProcessorValueTreeState& apvts)
{
    auto* obj = new juce::DynamicObject();
    for (auto* p : apvts.processor.getParameters())
    {
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            obj->setProperty (rp->getParameterID(), rp->convertTo0to1 (rp->getValue()));
    }
    auto state = apvts.copyState();
    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto child = state.getChild (i);
        if (child.hasType ("PARAM"))
        {
            const auto id = child.getProperty ("id").toString();
            if (auto* raw = apvts.getRawParameterValue (id))
                obj->setProperty (id, (double) raw->load());
        }
    }
    return juce::var (obj);
}

void PresetManager::applyJsonParams (juce::AudioProcessorValueTreeState& apvts, const juce::var& json)
{
    if (auto* obj = json.getDynamicObject())
    {
        for (const auto& prop : obj->getProperties())
        {
            if (auto* p = apvts.getParameter (prop.name.toString()))
            {
                if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (p))
                    choice->setValueNotifyingHost (choice->convertTo0to1 ((float) (int) prop.value));
                else if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
                    ranged->setValueNotifyingHost (ranged->convertTo0to1 ((float) prop.value));
            }
        }
    }
}

bool PresetManager::loadPresetFile (const juce::File& file, juce::AudioProcessorValueTreeState& apvts, juce::ValueTree& extraState)
{
    auto json = juce::JSON::parse (file.loadFileAsString());
    if (! json.isObject()) return false;
    if (auto params = json.getProperty ("parameters", {}))
        applyJsonParams (apvts, params);
    if (auto engine = json.getProperty ("engineIndex", {}))
        if (auto* p = apvts.getParameter ("engine"))
            p->setValueNotifyingHost (p->convertTo0to1 ((float) (int) engine));
    extraState = juce::ValueTree::fromXml (json.getProperty ("extra", "").toString());
    return true;
}

bool PresetManager::loadPreset (int index, juce::AudioProcessorValueTreeState& apvts, juce::ValueTree& extraState)
{
    if (index < 0 || index >= (int) presets_.size()) return false;
    currentIndex_ = index;
    const auto& info = presets_[(size_t) index];
    if (info.kind != LibraryItemKind::Preset)
        return true; // assets loaded by editor path
    if (info.file.existsAsFile())
        return loadPresetFile (info.file, apvts, extraState);
    return true;
}

bool PresetManager::savePreset (const juce::String& name, const juce::String& category,
                                juce::AudioProcessorValueTreeState& apvts, const juce::ValueTree& extraState)
{
    auto* root = new juce::DynamicObject();
    root->setProperty ("schema", 1);
    root->setProperty ("name", name);
    root->setProperty ("category", category);
    root->setProperty ("author", "User");
    root->setProperty ("mood", "user");
    root->setProperty ("energy", 0.5);
    root->setProperty ("brightness", 0.5);
    root->setProperty ("warmth", 0.5);
    root->setProperty ("artworkSeed", name.hashCode());
    if (auto* eng = apvts.getRawParameterValue ("engine"))
        root->setProperty ("engineIndex", (int) eng->load());
    root->setProperty ("parameters", paramsToJson (apvts));
    if (auto xml = extraState.createXml())
        root->setProperty ("extra", xml->toString());

    auto file = userDir_.getChildFile (name + ".json");
    file.replaceWithText (juce::JSON::toString (juce::var (root), true));
    rescan();
    for (int i = 0; i < (int) presets_.size(); ++i)
        if (presets_[(size_t) i].name == name)
            currentIndex_ = i;
    return true;
}

void PresetManager::next()
{
    if (presets_.empty()) return;
    // Skip non-presets for prev/next navigation
    for (int n = 0; n < (int) presets_.size(); ++n)
    {
        currentIndex_ = (currentIndex_ + 1) % (int) presets_.size();
        if (presets_[(size_t) currentIndex_].kind == LibraryItemKind::Preset)
            return;
    }
}

void PresetManager::previous()
{
    if (presets_.empty()) return;
    for (int n = 0; n < (int) presets_.size(); ++n)
    {
        currentIndex_ = (currentIndex_ - 1 + (int) presets_.size()) % (int) presets_.size();
        if (presets_[(size_t) currentIndex_].kind == LibraryItemKind::Preset)
            return;
    }
}

juce::StringArray PresetManager::expandQueryTokens (const juce::String& query)
{
    juce::StringArray tokens;
    auto q = query.toLowerCase().trim();
    if (q.isEmpty()) return tokens;

    // Phrase → synonym bags (deterministic NL-ish search)
    struct Map { const char* phrase; const char* expand; };
    static constexpr Map maps[] = {
        { "warm tape piano", "warm piano keys tape" },
        { "hans zimmer brass", "epic cinematic brass fanfare" },
        { "blade runner", "dark pad cyber neon atmosphere" },
        { "dark horror choir", "gothic dark choir vocal haunting" },
        { "moog bass", "moog bass growl sub" },
        { "trap reese", "trap reese bass dark" },
        { "ambient pad", "ambient pad atmosphere ethereal" },
        { "acid", "acid bass line" },
    };

    for (const auto& m : maps)
    {
        if (q.contains (m.phrase))
        {
            tokens.addTokens (m.expand, " ", "");
            break;
        }
    }

    tokens.addTokens (q, " ,;-/", "");
    tokens.trim();
    tokens.removeEmptyStrings();
    return tokens;
}

std::vector<int> PresetManager::filteredIndices (const juce::String& category, const juce::String& query,
                                                 bool favoritesOnly) const
{
    std::vector<int> out;
    const auto tokens = expandQueryTokens (query);
    const bool filterCollection = category.isNotEmpty() && category != "All"
                                  && category != "Favorites" && category != "Factory";

    for (int i = 0; i < (int) presets_.size(); ++i)
    {
        const auto& p = presets_[(size_t) i];
        if (favoritesOnly || category == "Favorites")
            if (! isFavorite (p.name)) continue;

        if (filterCollection)
        {
            const bool catMatch = p.category.equalsIgnoreCase (category)
                                  || collectionForCategory (p.category).equalsIgnoreCase (category);
            if (! catMatch) continue;
        }

        if (! tokens.isEmpty())
        {
            bool match = false;
            auto hay = (p.name + " " + p.category + " " + p.engine + " " + p.mood
                        + " " + p.author + " " + p.tags.joinIntoString (" ")).toLowerCase();
            for (const auto& t : tokens)
            {
                if (hay.contains (t))
                {
                    match = true;
                    break;
                }
            }
            if (! match) continue;
        }
        out.push_back (i);
    }
    return out;
}

juce::String PresetManager::serializePresetJson (const juce::DynamicObject::Ptr& root)
{
    return juce::JSON::toString (juce::var (root.get()), true);
}

juce::var PresetManager::parsePresetJson (const juce::String& text)
{
    return juce::JSON::parse (text);
}
