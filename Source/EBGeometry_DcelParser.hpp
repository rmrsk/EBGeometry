/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DcelParser.hpp
  @brief  Declaration of utilities for passing data into DCEL structures. 
  @author Robert Marskar
*/

#ifndef EBGeometry_DcelParser
#define EBGeometry_DcelParser

// Std includes
#include <vector>
#include <memory>
#include <map>

// Our includes
#include "EBGeometry_DcelVertex.hpp"
#include "EBGeometry_DcelEdge.hpp"
#include "EBGeometry_DcelFace.hpp"
#include "EBGeometry_DcelMesh.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace Dcel {

  /*!
    @brief Namespace which encapsulates possible file parsers for building DCEL meshes
  */
  namespace Parser {
  
    /*!
      @brief Class for generation a Dcel::MeshT<T> from the Stanford PLY file format
      @note T is the precision used for storing the mesh. 
    */
    template <class T>
    class PLY {
    public:

      /*!
	@brief Alias for cutting down on typing
      */
      using Vertex = VertexT<T>;

      /*!
	@brief Alias for cutting down on typing
      */      
      using Edge = EdgeT<T>;

      /*!
	@brief Alias for cutting down on typing
      */      
      using Face = FaceT<T>;

      /*!
	@brief Alias for cutting down on typing
      */            
      using Mesh = MeshT<T>;

      /*!
	@brief Alias for cutting down on typing
      */            
      using EdgeIterator = EdgeIteratorT<T>;

      /*!
	@brief Static function which reads an ASCII .ply file and returns a DCEL mesh. 
	@param[in]  a_filename File name
      */
      inline
      static std::shared_ptr<Mesh> readASCII(const std::string a_filename);      

      /*!
	@brief Static function which reads an ASCII .ply file and puts it in a mesh. 
	@param[out] a_mesh     DCEL mesh 
	@param[in]  a_filename File name
      */
      inline
      static void readASCII(Mesh& a_mesh, const std::string a_filename);



    protected:

      /*!
	@brief Read an ASCII header
	@details This reads the number of vertices and faces in the PLY file. Note that it only reads the header. 
	@param[out]   a_numVertices Number of vertices
	@param[out]   a_numFaces    Number of faces
	@param[inout] a_inputStream File stream. On output, the filestream is at the end of the PLY header. 
      */
      inline
      static void readHeaderASCII(int&           a_numVertices,
				  int&           a_numFaces,
				  std::ifstream& a_inputStream);

      /*!
	@brief Read ASCII vertices. 
	@param[out] a_vertices DCEL vertices. These are constructed in this routine.
	@param[in]  a_numVertices Number of vertices to read
	@param[in]  a_inputStream Input file stream. 
	@note The next getline() from a_inputStream must read the first vertex (i.e. don't rewind the stream before entering this routine)
      */
      inline
      static void readVerticesASCII(std::vector<std::shared_ptr<Vertex> >& a_vertices,
				    const int                              a_numVertices,
				    std::ifstream&                         a_inputStream);

      /*!
	@brief Read ASCII faces and create mesh connectivity. 
	@param[out] a_faces DCEL faces. Constructured in this routine. 
	@param[out] a_edges DCEL edges. Constructured in this routine. 
	@param[out] a_vertices DCEL edges. Constructured in readVerticesASCII. 
	@note The next getline() from inputStream must read the first face, i.e. we assume that read_ascii_vertices was called IMMEDIATELY before this function. 
	That function will center the fstream on the correct line in the input file. 
      */
      inline
      static void readFacesASCII(std::vector<std::shared_ptr<Face> >&         a_faces,
				 std::vector<std::shared_ptr<Edge> >&         a_edges,
				 const std::vector<std::shared_ptr<Vertex> >& a_vertices,
				 const int                                    a_numFaces,
				 std::ifstream&                               a_inputStream);

      /*!
	@brief Reconcile pair edges, i.e. find the pair edge for every constructed half-edge
	@param[inout] a_edges Half edges. 
      */
      inline
      static void reconcilePairEdges(std::vector<std::shared_ptr<Edge> >& a_edges);
    };
  }
}

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_DcelParserImplem.hpp"

#endif
