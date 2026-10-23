#pragma once
#include <string>
#include <format>
#include <ultimaille/all.h>

using namespace UM;

struct Attribute {

    enum ElementKind {
        POINTS_ELT = 0,
        CORNERS_ELT = 1,
        EDGES_ELT = 2,
        FACETS_ELT = 3,
        CELLS_ELT = 4,
        CELL_FACETS_ELT = 5,
        CELL_CORNERS_ELT = 6,
        ELEMENT_KIND_COUNT
    };

    enum ElementType {
        DOUBLE_ELT,
        INT_ELT,
        BOOL_ELT,
        VEC2_ELT,
        VEC3_ELT,
        ELEMENT_TYPE_COUNT
    };

    std::string name;
    ElementKind kind;
    ElementType type;
    std::shared_ptr<ContainerBase> ptr;

    std::string getFullName() const {
        return std::format("{}.{}", elementKindToString(kind), name);
        // return std::format("{}.{}{}",
        // 	elementKindToString(kind),
        // 	name,
        // 	// elementTypeToString(type),
        // 	getNDims() > 1 && dim >= 0 ? std::format("[{}]", dim) : "");
    }
	bool operator==(const Attribute& other) const {
		return name == other.name &&
				kind == other.kind &&
				type == other.type; /*&&*/
				// dim == other.dim;
	}

	bool operator!=(const Attribute& other) const {
		return !(*this == other);
	}

    static std::vector<Attribute> getAttributes(SurfaceAttributes &attr) {
        
        std::vector<Attribute> attributes;
        for (auto &[kind, c] : getAttributeContainers(attr)) {
            auto attrs = getAttributesFromContainer(kind, c);
            for (auto a : attrs) {
                attributes.push_back(a);
            }
        }
        return attributes;

    }

    static std::optional<Attribute> getAttribute(SurfaceAttributes &attr, const std::string fullname) {

        for (auto &attr : getAttributes(attr)) {
            if (attr.getFullName() == fullname)
                return attr;
        }

        return std::nullopt;
    }

    static std::vector<std::pair<ElementKind, NamedContainer>> getAttributeContainers(SurfaceAttributes &attr) {
        std::vector<std::pair<ElementKind, NamedContainer>> containers;
        
        for (auto &c : attr.points)
            containers.push_back({ElementKind::POINTS_ELT, c});
        for (auto &c : attr.corners)
            containers.push_back({ElementKind::CORNERS_ELT, c});
        for (auto &c : attr.facets)
            containers.push_back({ElementKind::FACETS_ELT, c});

        return containers;
    }

    static std::vector<Attribute> getAttributesFromContainer(ElementKind kind, NamedContainer &container) {
        std::vector<Attribute> attributes;
        
        // Get the type of the container
        ElementType type = ElementType::DOUBLE_ELT; // Default type
        if (auto a = dynamic_cast<AttributeContainer<double>*>(container.ptr.get())) {
            type = ElementType::DOUBLE_ELT;
        } else if (auto a = dynamic_cast<AttributeContainer<int>*>(container.ptr.get())) {
            type = ElementType::INT_ELT;
        } else if (auto a = dynamic_cast<AttributeContainer<bool>*>(container.ptr.get())) {
            type = ElementType::BOOL_ELT;
        // } else if (auto a = dynamic_cast<AttributeContainer<vec2>*>(container.ptr.get())) {
        //     type = ElementType::VEC2_ELT;
        //     attributes.emplace_back(container.name + "[0]", kind, ElementType::DOUBLE_ELT, container.ptr, true, 0);
        //     attributes.emplace_back(container.name + "[1]", kind, ElementType::DOUBLE_ELT, container.ptr, true, 1);
        // } else if (auto a = dynamic_cast<AttributeContainer<vec3>*>(container.ptr.get())) {
        //     type = ElementType::VEC3_ELT;
        //     attributes.emplace_back(container.name + "[0]", kind, ElementType::DOUBLE_ELT, container.ptr, true, 0);
        //     attributes.emplace_back(container.name + "[1]", kind, ElementType::DOUBLE_ELT, container.ptr, true, 1);
        //     attributes.emplace_back(container.name + "[2]", kind, ElementType::DOUBLE_ELT, container.ptr, true, 2);
        } else {
            throw std::runtime_error("Unknown attribute type for container: " + container.name);
        }

        // attributes.emplace_back(container.name, kind, type, container.ptr, false, -1);
        attributes.emplace_back(container.name, kind, type, container.ptr);

        return attributes;
    }

    static std::string elementKindToString(ElementKind e) {
        switch (e) {
            case ElementKind::POINTS_ELT: return "points";
            case ElementKind::EDGES_ELT: return "edges";
            case ElementKind::FACETS_ELT: return "facets";
            case ElementKind::CELL_FACETS_ELT: return "cell_facets";
            case ElementKind::CORNERS_ELT: return "corners";
            case ElementKind::CELL_CORNERS_ELT: return "cell_corners";
            case ElementKind::CELLS_ELT: return "cells";
            default: return "unknown";
        }
    }

    static std::string elementTypeToString(ElementType t) {
        switch (t) {
            case ElementType::DOUBLE_ELT: return "double";
            case ElementType::INT_ELT: return "int";
            case ElementType::BOOL_ELT: return "bool";
            case ElementType::VEC2_ELT: return "vec2";
            case ElementType::VEC3_ELT: return "vec3";
            default: return "unknown";
        }
    }


};