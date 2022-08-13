#include <cstdint>
#include <iostream>
#include <vector>

#define CONCATENATE_IMPLEMENTATION(X, Y) X##Y
#define CONCATENATE(X, Y) CONCATENATE_IMPLEMENTATION(X, Y)

#define PROPERTY(Type, Name, Description) EditorProperty CONCATENATE(_, __LINE__) = { sizeof(EditorProperty), alignof(Type), Name, Description, std::make_unique<PropertyInspector<Type>>() };

class PropertyInspectorInterface
{
public:
	virtual ~PropertyInspectorInterface() = default;

	virtual void showProperty(std::uintptr_t) const = 0;
	virtual void editProperty(std::uintptr_t) const = 0;
};

template <typename PropertyType>
class PropertyInspector final : public PropertyInspectorInterface
{
public:
	void showProperty(std::uintptr_t propertyAddress) const override
	{
		std::cout << "\nProperty value: ";
		std::cout << *reinterpret_cast<PropertyType*>(propertyAddress) << std::endl;
	}

	void editProperty(std::uintptr_t propertyAddress) const override
	{
		std::cout << "Enter new value: ";
		std::cin >> *reinterpret_cast<PropertyType*>(propertyAddress);
	}
};

class PropertyData
{
public:
	std::uintptr_t address;
	size_t size;
	size_t alignment;
	std::string_view name;
	std::string_view description;
	std::unique_ptr<PropertyInspectorInterface> inspector;
};

class Editor
{
	using PropertyList = std::vector<PropertyData>;

public:
	template <typename ... Types>
	static void CreateProperty(Types&& ... args)
	{
		properties.push_back({std::forward<Types>(args)...});
	}

	static void SelectObject(void* objectPtr, size_t objectSize)
	{
		const auto from = reinterpret_cast<uintptr_t>(objectPtr);
		const auto to = from + objectSize;

		for (const auto& propertyData : properties)
		{
			const auto address = propertyData.address;
			if (address >= from && address <= to)
			{
				std::cout << '\n' << propertyData.name << " (" << propertyData.description << ')' << std::endl;

				auto variable = address;
				variable += propertyData.size;

				if (variable % propertyData.alignment != 0)
				{
					variable += propertyData.alignment;
					variable -= (variable % propertyData.alignment);
				}

				propertyData.inspector->showProperty(variable);
				propertyData.inspector->editProperty(variable);
			}
		}
	}

	static void RemoveProperty(std::uintptr_t address)
	{
		properties.erase(std::remove_if(properties.begin(), properties.end(),
			[address](const auto& propertyData)
			{
				return propertyData.address == address;
			}
		), properties.end());
	}

private:
	inline static PropertyList properties;
};

class EditorProperty final
{
public:
	template <typename ... Types>
	EditorProperty(Types&& ... arguments)
	{
		Editor::CreateProperty(reinterpret_cast<uintptr_t>(this), std::forward<Types>(arguments)...);
	}

	~EditorProperty()
	{
		Editor::RemoveProperty(reinterpret_cast<uintptr_t>(this));
	}
};

class Model
{
public:
	PROPERTY(std::string, "Model Id", "")
	std::string modelId;
};

class Sprite
{
public:
	PROPERTY(int, "X", "Sprite X axis position")
	int x = 0;

	PROPERTY(int, "Y", "Sprite Y axis position")
	int y = 0;

	PROPERTY(float, "Scale X", "Sprite X axis scale")
	float xScale = 1.0f;

	PROPERTY(float, "Scale Y", "Sprite Y axis scale")
	float yScale = 1.0f;

	PROPERTY(std::string, "Texture Id", "Sprite texture Id")
	std::string textureId = "DebugTexture.png";
};

int main (int argc, char* argv[])
{
	Sprite playerSprite;
	Sprite enemySprite;
	Model playerModel;

	Editor::SelectObject(&enemySprite, sizeof(enemySprite));

	return 0;
}