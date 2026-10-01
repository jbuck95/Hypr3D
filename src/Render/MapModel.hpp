#pragma once

#include "World/Camera.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace H3D {

// A glTF 2.0 map (.glb or .gltf) placed in the room: GL meshes, decoded
// textures and world-space collision triangles. Owns its GL objects; load()
// and destroy() must run with the EGL context current (they are called from
// GLScene::render, like the panorama refresh).
class CMapModel {
  public:
    // World-space triangle (the map transform is already applied). This is
    // what collision consumes.
    struct STL {
        Vec3 a, b, c;
    };

    // rotationDeg is XYZ Euler angles in degrees. Re-loading with the same
    // path and transform is a cheap no-op (mtime is NOT checked here -- the
    // caller decides when the file changed).
    bool load(const std::string& path, const Vec3& position,
              const Vec3& rotationDeg, const Vec3& scale);
    void destroy();

    bool loaded() const {
        return m_loaded;
    }

    // The map's own transform, remembered for the draw pass.
    void setTransform(const Vec3& position, const Vec3& rotationDeg,
                      const Vec3& scale);

    // Global emissive multiplier (config); 0 = pure lambert, 1 = as authored.
    void setEmissiveScale(float s) {
        m_emissiveScale = s > 0.f ? s : 0.f;
    }

    // Baked-map mode (default): textures carry all lighting, so the shader
    // shows albedo/emission as-is with no dynamic light -- a lambert term
    // re-shades baked shadows and greys out down-facing light fixtures.
    void setFlat(bool flat) {
        m_flat = flat;
    }

    // cameraPos orders the blend pass far-to-near (translucent surfaces
    // need back-to-front to composite correctly against each other).
    void draw(const Mat4& vp, const Vec3& cameraPos) const;

    // Wireframe of the world-space collision triangles (red, x-ray). The
    // line buffer is (re)built on the next draw whenever the triangles were
    // recomputed (m_debugPending) -- scale/rotation changes included.
    void setDebugCollisions(bool on);
    void drawDebug(const Mat4& vp);

    const std::vector<STL>& triangles() const {
        return m_triangles;
    }

    // Bump this after every (re)load so collision can rebuild its BVH.
    uint32_t generation() const {
        return m_generation;
    }

  private:
    // Node-space collision triangles; the world-space set (m_triangles) is
    // recomputed from these whenever the config transform changes.
    void recomputeTriangles();

    struct SPrimitive {
        unsigned int vao = 0, vbo = 0, ebo = 0;
        int          count = 0;   // index count (or vertex count unindexed)
        bool         indexed = false;
        unsigned int texture = 0; // 0 = untextured (white fallback)
        float        color[4] = {1.f, 1.f, 1.f, 1.f};
        unsigned int emissiveTex = 0;
        float        emissiveFactor[3] = {1.f, 1.f, 1.f};
        float        emissiveStrength = 1.0f;

        // glTF alphaMode: 0 opaque, 1 mask (uAlphaCutoff), 2 blend.
        int          alphaMode = 0;
        float        alphaCutoff = 0.5f;
        Vec3         centroid{}; // node-space AABB center, for blend sorting
    };

    unsigned int uploadTexture(const std::string& modelDir, const void* image,
                               bool embedded, const void* data, size_t size);

    std::vector<SPrimitive> m_primitives;
    std::vector<unsigned>   m_ownedTextures; // unique image textures
    std::vector<STL>        m_localTriangles; // node space
    std::vector<STL>        m_triangles;      // world space (recomputed)
    std::string             m_path;
    Vec3                    m_position{};
    Vec3                    m_rotationDeg{};
    Vec3                    m_scale{1.0f, 1.0f, 1.0f};
    float                   m_emissiveScale = 1.0f;
    bool                    m_flat = true;
    bool                    m_loaded = false;
    uint32_t                m_generation = 0;

    // Collision wireframe.
    unsigned int            m_debugVAO = 0, m_debugVBO = 0;
    int                     m_debugVerts = 0;
    bool                    m_debugOn = false;
    bool                    m_debugPending = false; // triangles changed since build
    unsigned int            m_debugBuiltGen = 0;

    unsigned int            m_debugProgram = 0;
    int                     m_debugMVP = -1;

    unsigned int            m_program = 0;
    unsigned int            m_whiteTexture = 0;
    int                     m_uMVP = -1;
    int                     m_uModel = -1;
    int                     m_uColor = -1;
    int                     m_uTex = -1;
    int                     m_uHasTex = -1;
    int                     m_uEmissive = -1;
    int                     m_uHasEmissive = -1;
    int                     m_uEmissiveFactor = -1;
    int                     m_uEmissiveStrength = -1;
    int                     m_uFlat = -1;
    int                     m_uAlphaMode = -1;
    int                     m_uAlphaCutoff = -1;
};

} // namespace H3D
