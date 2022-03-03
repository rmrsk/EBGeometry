/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DcelParserImplem.hpp
  @brief  Implementation of EBGeometry_DcelParser.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_DcelParserImplem
#define EBGeometry_DcelParserImplem

// Std includes
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>

// Our includes
#include "EBGeometry_DcelParser.hpp"
#include "EBGeometry_DcelVertex.hpp"
#include "EBGeometry_DcelEdge.hpp"
#include "EBGeometry_DcelFace.hpp"
#include "EBGeometry_DcelMesh.hpp"
#include "EBGeometry_DcelIterator.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace Dcel {

  template <class T>
  inline
  std::shared_ptr<EBGeometry::Dcel::MeshT<T> > Parser::PLY<T>::readASCII(const std::string a_filename) {
    auto mesh = std::make_shared<Mesh>();

    readASCII(*mesh, a_filename);

    return mesh;
  }

  template <class T>
  inline
  void Parser::PLY<T>::readASCII(Mesh& a_mesh, const std::string a_filename) {
    std::ifstream filestream(a_filename);

    if(filestream.is_open()){
      std::vector<std::shared_ptr<Vertex> >& vertices  = a_mesh.getVertices();
      std::vector<std::shared_ptr<Edge> >& edges       = a_mesh.getEdges();
      std::vector<std::shared_ptr<Face> >& faces       = a_mesh.getFaces();

      vertices.resize(0);
      edges.resize(0);
      faces.resize(0);

      int numVertices;  // Number of vertices
      int numFaces;     // Number of faces

      Dcel::Parser::PLY<T>::readHeaderASCII(numVertices, numFaces, filestream);
      Dcel::Parser::PLY<T>::readVerticesASCII(vertices, numVertices, filestream);
      Dcel::Parser::PLY<T>::readFacesASCII(faces, edges, vertices, numFaces, filestream);
      Dcel::Parser::PLY<T>::reconcilePairEdges(edges);

      a_mesh.sanityCheck();
    
      filestream.close();

      a_mesh.reconcile(EBGeometry::Dcel::MeshT<T>::VertexNormalWeight::Angle);
    }
    else{
      const std::string error = "Dcel::Parser::PLY::readASCII - ERROR! Could not open file " + a_filename;
      std::cerr << error + "\n";
    }
  }

  template <class T>
  inline
  void Parser::PLY<T>::readHeaderASCII(int&           a_numVertices,
				       int&           a_numFaces,
				       std::ifstream& a_inputStream) {

    std::string str1;
    std::string str2;
    std::string line;

    // Get number of vertices
    a_inputStream.clear();
    a_inputStream.seekg(0);
    while (getline(a_inputStream, line)){
      std::stringstream sstream(line);
      sstream >> str1 >> str2 >> a_numVertices;
      if(str1 == "element" && str2 == "vertex"){
	break;
      }
    }

    // Get number of faces
    a_inputStream.clear();
    a_inputStream.seekg(0);
    while (getline(a_inputStream, line)){
      std::stringstream sstream(line);
      sstream >> str1 >> str2 >> a_numFaces;
      if(str1 == "element" && str2 == "face"){
	break;
      }
    }

    // Find the line # containing "end_header" halt the input stream there
    a_inputStream.clear();
    a_inputStream.seekg(0);
    while (getline(a_inputStream, line)){
      std::stringstream sstream(line);
      sstream >> str1;
      if(str1 == "end_header"){
	break;
      }
    }
  }

  template <class T>
  inline
  void Parser::PLY<T>::readVerticesASCII(std::vector<std::shared_ptr<Vertex> >& a_vertices,
					 const int                              a_numVertices,
					 std::ifstream&                         a_inputStream) {

    Vec3T<T> pos;
    T& x = pos[0];
    T& y = pos[1];
    T& z = pos[2];

    Vec3T<T> norm;
    T& nx = norm[0];
    T& ny = norm[1];
    T& nz = norm[2];
  
    int num = 0;

    std::string line;
    while(std::getline(a_inputStream, line)){
      std::stringstream sstream(line);
      sstream >> x >> y >> z >> nx >> ny >> nz;

      a_vertices.emplace_back(std::make_shared<Vertex>(pos, norm));

      // We have read all the vertices we should read. Exit now -- after this the inputStream will begin reading faces. 
      num++;
      if(num == a_numVertices) break;
    }
  }

  template <class T>
  inline
  void Dcel::Parser::PLY<T>::readFacesASCII(std::vector<std::shared_ptr<Face> >&         a_faces,
					    std::vector<std::shared_ptr<Edge> >&         a_edges,
					    const std::vector<std::shared_ptr<Vertex> >& a_vertices,
					    const int                                    a_numFaces,
					    std::ifstream&                               a_inputStream) {
    int numVertices;
    std::vector<int> vertexIndices;

    std::string line;
    int counter = 0;
    while(std::getline(a_inputStream, line)){
      counter++;
    
      std::stringstream sstream(line);

      sstream >> numVertices;
      vertexIndices.resize(numVertices);
      for (int i = 0; i < numVertices; i++){
	sstream >> vertexIndices[i];
      }

      if(numVertices < 3) std::cerr << "Dcel::Parser::PLY::readFacesASCII - a face must have at least three vertices!\n";
    
      // Get the vertices that make up this face. 
      std::vector<std::shared_ptr<Vertex> > curVertices;
      for (int i = 0; i < numVertices; i++){
	const int vertexIndex = vertexIndices[i];
	curVertices.emplace_back(a_vertices[vertexIndex]);
      }

      // Build inside half edges and give each vertex an outgoing half edge. This may get overwritten later,
      // but the outgoing edge is not unique so it doesn't matter. 
      std::vector<std::shared_ptr<Edge> > halfEdges;
      for (const auto& v : curVertices){
	halfEdges.emplace_back(std::make_shared<Edge>(v));
	v->setEdge(halfEdges.back());
      }

      a_edges.insert(a_edges.end(), halfEdges.begin(), halfEdges.end());

      // Associate next/previous for the half edges inside the current face. Wish we had a circular iterator
      // but this will have to do. 
      for (int i = 0; i < halfEdges.size(); i++){
	auto& curEdge  = halfEdges[i];
	auto& nextEdge = halfEdges[(i+1)%halfEdges.size()];

	curEdge->setNextEdge(nextEdge);
	nextEdge->setPreviousEdge(curEdge);
      }

      // Construct a new face
      a_faces.emplace_back(std::make_shared<Face>(halfEdges.front()));
      auto& curFace = a_faces.back();

      // Half edges get a reference to the currently created face
      for (auto& e : halfEdges){
	e->setFace(curFace);
      }

      // Must give vertices access to all faces associated with them since PLY files do not give any edge association. 
      for (auto& v : curVertices){
	v->addFace(curFace);
      }

    
      if(counter == a_numFaces) break;
    }
  }

  template <class T>
  inline
  void Parser::PLY<T>::reconcilePairEdges(std::vector<std::shared_ptr<Edge> >& a_edges) {
    for (auto& curEdge : a_edges){
      const auto& nextEdge = curEdge->getNextEdge();
    
      const auto& vertexStart = curEdge->getVertex();
      const auto& vertexEnd   = nextEdge->getVertex();

      for (const auto& p : vertexStart->getFaces()){
	for (EdgeIterator edgeIt(*p); edgeIt.ok(); ++edgeIt){
	  const auto& polyCurEdge  = edgeIt();
	  const auto& polyNextEdge = polyCurEdge->getNextEdge();

	  const auto& polyVertexStart = polyCurEdge->getVertex();
	  const auto& polyVertexEnd   = polyNextEdge->getVertex();

	  if(vertexStart == polyVertexEnd && polyVertexStart == vertexEnd){ // Found the pair edge
	    curEdge->setPairEdge(polyCurEdge);
	    polyCurEdge->setPairEdge(curEdge);
	  }
	}
      }
    }
  }
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
