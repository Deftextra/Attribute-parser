#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>


using namespace std;

// TODO: Add more cases for errors.
// TODO: Refactor Data structures.
struct Tag {
  using attribute = std::string;
  using value = std::string;

  std::string name;
  bool isClosing;


  std::map<attribute, value> attributes;

  std::vector<Tag *> containingTags;

  Tag(std::string name, bool isClosing = false)
    : name{name}, isClosing{isClosing} {}
};

class TagParseTree
{
  public:
    Tag *root;
    TagParseTree()
      :root{NULL} {}

    explicit TagParseTree(std::string name) : root{new Tag(name)} {}

    bool isEmpty()
    {
      return root;
    }

    Tag *search(const std::string &name) {  return search(root, name); }


    Tag *insert(std::string target, Tag *add) 
    {
      Tag *fTarget = search(target);

      if (fTarget)
      {
        fTarget->containingTags.push_back(add);
        return fTarget;
      }
      return NULL;
    }
  private:
    // We can insert by tag names because tag names are unique (See Readme)
    Tag *search(Tag * h, const std::string &name)
    {

      if (h->name == name)
        return h;

      if (!h->containingTags.empty())
      {
        for (auto& tag : h->containingTags)
        {
          if (search(tag,name))
            return search(tag, name);
        }
      }

      return NULL;
    }

};

Tag* parseTag(std::string openingTag) {
  stringstream ss(openingTag);
  std::string tagName;
  char c;

  // Is Opening Tag
  if (ss >> c && c == '<' && ss.peek() != '/') {
    ss >> tagName;

    // Has no attributes
    if (tagName.back() == '>') {
      tagName.pop_back();
      ss.putback('>');
    }

    Tag *oTag = new Tag{tagName};

    std::string attributeName;

    while (ss >> attributeName && ss >> c && c == '=') {
      std::string attributeValue;

      if (ss >> c && c == '"') {
        while (ss >> c && c != '"')
          attributeValue += c;
      }
      oTag->attributes.insert({attributeName, attributeValue});
    }

    if ((c == '"' || c == '<') && attributeName == ">")
      return oTag;

    return std::nullptr_t{};
  }

  // Is closing Tag
  if (ss >> c && c == '/') {
    std::string tagName;

    while (ss >> c && c != '>')
      tagName += c;

    return new Tag{tagName, true};
  }

  return std::nullptr_t{};
}

int parseHtlmr(std::istream &s, const std::string rootName, TagParseTree &t, int htlmrSize)
{

  std::string childLine;

  Tag *ct;

  while (std::getline(s, childLine) && (ct = parseTag(childLine)) && htlmrSize--)
  {

    if (ct->isClosing && ct->name != rootName)
    {
      /// error occured;
      break;
    }

    if (!ct->isClosing)
    {
      // If we find an opening tag this tag must be a direct child tag of root.
      t.insert(rootName, ct);

      // If allong the way we find other opening tags we want to find their closing tags aswell
      htlmrSize = parseHtlmr(s, ct->name, t, htlmrSize); 
    }

    // Found the closing tag so we stop
    if (ct->isClosing && ct->name == rootName)
      break;
  }

  return htlmrSize;
}

Tag* findContaingTag(Tag* rootTag, std::string tagName)
{
  for (auto& tag : rootTag->containingTags)
  {
    if (tag->name == tagName)
      return tag;
  }

  return NULL;
}



// TODO: Refactor this function
void queries(std::istream& s, int querySize,TagParseTree& t)
{

  std::string query;

  while (getline(s, query) && querySize--)
  {
    // Refactor this code!!! very ugly
    std::string combinedTagNames;
    std::string attributeName;
    std::string tagName;
    stringstream sQuery(query);

    getline(sQuery, combinedTagNames, '~');
    sQuery >> attributeName;


    //std::cout << combinedTagNames << std::endl;
    std::stringstream sTagNames(combinedTagNames);
    Tag* tag = t.search("global");


    // Find tag in htlmr
    while (getline(sTagNames, tagName, '.'))
    {
      tag = findContaingTag(tag, tagName);

      if (!tag)
      {
        break;
      }
    }

    if (!sTagNames.eof() && tag)
    {
      // Error occured
      continue;
    }

    if (!sTagNames.eof() && !tag)
    {
      std::cout << "Not Found!" << std::endl;
      continue;
    }

    if ((!tag && sTagNames.eof()))
    {
      std::cout << "Not Found" << std::endl;
      continue;
    }
    //     Find attribute in Tag
    if (tag && sTagNames.eof())
    {
      if (!tag->attributes[attributeName].empty())
        std::cout << tag->attributes[attributeName] << std::endl;
      else
        std::cout << "Not Found" << std::endl;
    }
  }

}

int main()
{

  //Set up Htlmr parse Tree
  const std::string globalRootName = "global";
  TagParseTree t {globalRootName};
  int htlmrSize, querySize;
  std::cin >> htlmrSize >> querySize;
  std::cin.ignore();  

  if (!parseHtlmr(std::cin, globalRootName, t, htlmrSize))
  {
    std::cerr << "Parse error occured" << std::endl;
    return 1;
  }

  queries(std::cin, querySize, t);

  return 0;

}

