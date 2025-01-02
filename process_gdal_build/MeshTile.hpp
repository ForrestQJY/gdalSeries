#ifndef MESHTILE_HPP
#define MESHTILE_HPP


#include "config.hpp"
#include "Mesh.hpp"
#include "TileCoordinate.hpp"
#include "Tile.hpp"
#include "GBOutputStream.hpp"

namespace gb {
	class MeshTile;
}

class GB_DLL gb::MeshTile :
	public Tile
{
	friend class MeshTiler;

public:

	/// Create an empty mesh tile object
	MeshTile();

	/// Create a mesh tile from a tile coordinate
	MeshTile(const TileCoordinate& coord);

	/// Write terrain data to the filesystem
	void
		writeFile(const char* fileName, bool writeVertexNormals) const;

	/// Write terrain data to an output stream
	void
		writeFile(GBOutputStream& ostream, bool writeVertexNormals) const;

	/// Does the terrain tile have child tiles?
	bool
		hasChildren() const;

	/// Does the terrain tile have a south west child tile?
	bool
		hasChildSW() const;

	/// Does the terrain tile have a south east child tile?
	bool
		hasChildSE() const;

	/// Does the terrain tile have a north west child tile?
	bool
		hasChildNW() const;

	/// Does the terrain tile have a north east child tile?
	bool
		hasChildNE() const;

	/// Specify that there is a south west child tile
	void
		setChildSW(bool on = true);

	/// Specify that there is a south east child tile
	void
		setChildSE(bool on = true);

	/// Specify that there is a north west child tile
	void
		setChildNW(bool on = true);

	/// Specify that there is a north east child tile
	void
		setChildNE(bool on = true);

	/// Specify that all child tiles are present
	void
		setAllChildren(bool on = true);

	/// Get the mesh data as a const object
	const gb::Mesh& getMesh() const;

	/// Get the mesh data
	gb::Mesh& getMesh();

protected:

	/// The terrain mesh data
	gb::Mesh mMesh;

private:

	char mChildren;               ///< The child flags

	/**
	 * @brief Bit flags defining child tile existence
	 *
	 * There is a good discussion on bitflags
	 * [here](http://www.dylanleigh.net/notes/c-cpp-tricks.html#Using_"Bitflags").
	 */
	enum Children {
		TERRAIN_CHILD_SW = 1,       // 2^0, bit 0
		TERRAIN_CHILD_SE = 2,       // 2^1, bit 1
		TERRAIN_CHILD_NW = 4,       // 2^2, bit 2
		TERRAIN_CHILD_NE = 8        // 2^3, bit 3
	};
};

#endif /* MESHTILE_HPP */
