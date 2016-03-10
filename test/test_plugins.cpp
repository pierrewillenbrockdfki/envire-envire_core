#include <boost/test/unit_test.hpp>
#include <envire_core/items/Item.hpp>
#include <Eigen/Geometry>
#include <class_loader/class_loader.h>
#include <envire_core/plugin/ClassLoader.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <vector>

using namespace envire::core;
using namespace envire;
using namespace std;

BOOST_AUTO_TEST_CASE(test_copy_operators)
{
    class StringItem : public envire::core::Item<std::string>
    {
        virtual std::string getClassName() const { return "StringItem"; }
    };

    StringItem string_item;
    string_item.setData("In-A-Gadda-Da-Vida");
    string_item.setTime(base::Time::now());
    string_item.setFrame("map_frame");

    // copy to new item
    StringItem string_item_copy;
    string_item_copy = string_item;
    BOOST_CHECK(string_item.getData() == string_item_copy.getData());
    BOOST_CHECK(string_item.getFrame() == string_item_copy.getFrame());
    BOOST_CHECK(string_item.getTime() == string_item_copy.getTime());
    BOOST_CHECK(string_item.getID() == string_item_copy.getID());

    // copy constructor
    StringItem string_item_copy_2(string_item);
    BOOST_CHECK(string_item.getData() == string_item_copy_2.getData());
    BOOST_CHECK(string_item.getFrame() == string_item_copy_2.getFrame());
    BOOST_CHECK(string_item.getTime() == string_item_copy_2.getTime());
    BOOST_CHECK(string_item.getID() == string_item_copy_2.getID());

    StringItem string_item_move;
    string_item_move = std::move(string_item_copy);
    BOOST_CHECK(string_item.getData() == string_item_move.getData());
    BOOST_CHECK(string_item.getFrame() == string_item_move.getFrame());
    BOOST_CHECK(string_item.getTime() == string_item_move.getTime());
    BOOST_CHECK(string_item.getID() == string_item_move.getID());

    StringItem string_item_move_2(std::move(string_item_copy_2));
    BOOST_CHECK(string_item.getData() == string_item_move_2.getData());
    BOOST_CHECK(string_item.getFrame() == string_item_move_2.getFrame());
    BOOST_CHECK(string_item.getTime() == string_item_move_2.getTime());
    BOOST_CHECK(string_item.getID() == string_item_move_2.getID());
}

BOOST_AUTO_TEST_CASE(vector_plugin_test)
{
    const char* root_path = std::getenv("AUTOPROJ_CURRENT_ROOT");
    string path_plugin;
    if(root_path != NULL)
    {
        path_plugin = string(root_path) + "/envire/envire_core/build/test/libenvire_vector_plugin.so";
    }

    BOOST_CHECK(path_plugin.length() > 0);
    class_loader::ClassLoader loader(path_plugin, false);

    // check available classes
    vector<string> classes = loader.getAvailableClasses<ItemBase>();
    BOOST_CHECK(classes.size() == 1);
    for(auto &class_name : classes)
        BOOST_CHECK(class_name == "VectorPlugin");

    // create instance
    boost::shared_ptr<ItemBase> vector_plugin = loader.createInstance<ItemBase>("VectorPlugin");
    BOOST_CHECK(vector_plugin->getClassName() == "VectorPlugin");

    // create another instance
    boost::shared_ptr<ItemBase> vector_plugin_2 = loader.createInstance<ItemBase>("VectorPlugin");
    BOOST_CHECK(vector_plugin->getID() != vector_plugin_2->getID());

    Item<Eigen::Vector3d>* vector_plugin_item = dynamic_cast< Item<Eigen::Vector3d>* >(vector_plugin.get());
    BOOST_CHECK(vector_plugin_item);

    Eigen::Vector3d data(1.0,2.0,3.0);
    vector_plugin_item->setData(data);
    BOOST_CHECK(vector_plugin_item->getData() == data);
}

BOOST_AUTO_TEST_CASE(class_loader_test)
{
    ClassLoader* loader = ClassLoader::getInstance();
    std::vector<std::string> xml_paths;
    const char* root_folder = std::getenv("AUTOPROJ_CURRENT_ROOT");
    BOOST_CHECK(root_folder != NULL);
    std::string root_folder_str(root_folder);
    root_folder_str += "/envire/envire_core/test";
    xml_paths.push_back(root_folder_str);
    loader->clear();
    loader->overridePluginXmlPaths(xml_paths);
    loader->reloadXMLPluginFiles();

    // load single element
    BOOST_CHECK(envire::core::ClassLoader::getInstance()->hasEnvireItem("VectorPlugin"));
    ItemBase::Ptr vector_plugin;
    BOOST_CHECK(envire::core::ClassLoader::getInstance()->createEnvireItem("VectorPlugin", vector_plugin));
    BOOST_CHECK(vector_plugin->getClassName() == "VectorPlugin");
    BOOST_CHECK(vector_plugin.use_count() == 1);
    ItemBase::Ptr vector_plugin_2 = vector_plugin;
    BOOST_CHECK(vector_plugin.use_count() == 2);
    BOOST_CHECK(vector_plugin->getID() == vector_plugin_2->getID());

    // load and cast element
    Item<Eigen::Vector3d>::Ptr vector_plugin_3;
    BOOST_CHECK(envire::core::ClassLoader::getInstance()->createEnvireItem< Item<Eigen::Vector3d> >("VectorPlugin", vector_plugin_3));
    BOOST_CHECK(vector_plugin_3->getClassName() == "VectorPlugin");
    BOOST_CHECK(vector_plugin_2->getID() != vector_plugin_3->getID());

    // create envire item by embedded type
    ItemBase::Ptr vector_plugin_4;
    BOOST_CHECK(envire::core::ClassLoader::getInstance()->createEnvireItemFor("Eigen::Vector3d", vector_plugin_4));
    BOOST_CHECK(vector_plugin_4->getClassName() == "VectorPlugin");

    // create a singleton plugin
    void* item_ptr = NULL;
    std::string test_string = "In-A-Gadda-Da-Vida";
    BOOST_CHECK(envire::core::ClassLoader::getInstance()->hasEnvireItem("StringPlugin"));
    {
        Item<std::string>::Ptr string_plugin;
        BOOST_CHECK(envire::core::ClassLoader::getInstance()->createEnvireItem< Item<std::string> >("StringPlugin", string_plugin));
        string_plugin->setData(test_string);
        item_ptr = string_plugin.get();
    }
    Item<std::string>::Ptr string_plugin;
    BOOST_CHECK(envire::core::ClassLoader::getInstance()->createEnvireItem< Item<std::string> >("StringPlugin", string_plugin));
    BOOST_CHECK(string_plugin->getData() == test_string);
    BOOST_CHECK(string_plugin.get() == item_ptr);
}