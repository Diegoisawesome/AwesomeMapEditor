﻿//////////////////////////////////////////////////////////////////////////////////
//
//
//                     d88b         888b           d888  888888888888
//                    d8888b        8888b         d8888  888
//                   d88''88b       888'8b       d8'888  888
//                  d88'  '88b      888 '8b     d8' 888  8888888
//                 d88Y8888Y88b     888  '8b   d8'  888  888
//                d88""""""""88b    888   '8b d8'   888  888
//               d88'        '88b   888    '888'    888  888
//              d88'          '88b  888     '8'     888  888888888888
//
//
// AwesomeMapEditor: A map editor for GBA Pokémon games.
// Copyright (C) 2016 Diegoisawesome, Pokedude
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// Include files
//
///////////////////////////////////////////////////////////
#include <AME/System/Configuration.hpp>
#include <AME/Widgets/OpenGL/AMEMapView.h>
#include <AME/Widgets/OpenGL/AMEEntityView.h>
#include <AME/Widgets/OpenGL/AMEBlockView.h>
#include <AME/Widgets/OpenGL/AMEOpenGLShared.hpp>
#include <QBoy/OpenGL/GLErrors.hpp>


namespace ame
{
    ///////////////////////////////////////////////////////////
    // Local definitions
    //
    ///////////////////////////////////////////////////////////
    #define MV_VERTEX_ATTR      0
    #define MV_COORD_ATTR       1

    ///////////////////////////////////////////////////////////
    // Local buffers
    //
    ///////////////////////////////////////////////////////////
    UInt8 pixelBuffer[64];
    UInt8 blockBuffer[256];


    ///////////////////////////////////////////////////////////
    // Function type:  Constructor
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    AMEMapView::AMEMapView(QWidget *parent)
        : QOpenGLWidget(parent),
          QOpenGLFunctions(),
          m_NPCBuffer(0u),
          m_IndexBuffer(0u),
          m_MoveTexture(0u),
          m_MoveBuffer(0u),
          m_PrimaryForeground(0),
          m_PrimaryBackground(0),
          m_SecondaryForeground(0),
          m_SecondaryBackground(0),
          m_ShowSprites(false),
          m_MovementMode(false),
          m_BlockView(0),
          m_FirstBlock(-1),
          m_LastBlock(-1),
          m_HighlightedBlock(-1),
          m_SelectSize(QSize(1,1)),
          m_CurrentTool(AMEMapView::Tool::None),
          m_CursorColor(Qt::GlobalColor::green),
          m_ShowCursor(false),
          m_IsInit(false)
    {
        QSurfaceFormat format = this->format();
        format.setDepthBufferSize(24);
        format.setStencilBufferSize(8);
        format.setVersion(3, 3);
        format.setProfile(QSurfaceFormat::CompatibilityProfile);
        setFormat(format);
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Destructor
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    AMEMapView::~AMEMapView()
    {
        if (m_IsInit)
            freeGL();
    }


    ///////////////////////////////////////////////////////////
    // Function type:  Virtual
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::initializeGL()
    {
        if (qboy::GLErrors::Current == NULL)
            qboy::GLErrors::Current = new qboy::GLErrors;

        // Initializes the OpenGL functions
        initializeOpenGLFunctions();

        glCheck(glGenBuffers(1, &m_MoveBuffer));
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_MoveBuffer));
        glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*16, NULL, GL_DYNAMIC_DRAW));

        QImage image(":/images/PermGL.png");
        glCheck(glGenTextures(1, &m_MoveTexture));
        glCheck(glBindTexture(GL_TEXTURE_2D, m_MoveTexture));
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 1024, 0, GL_BGRA, GL_UNSIGNED_BYTE, image.bits()));

        // Enables blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Virtual
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::resizeGL(int w, int h)
    {
        glCheck(glViewport(0, 0, w, h));
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Virtual
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::paintGL()
    {
        glCheck(glClearColor(240/255.0f, 240/255.0f, 240/255.0f, 1));
        glCheck(glClear(GL_COLOR_BUFFER_BIT));

        if (m_Maps.size() == 0)
            return;

        // Binds the vertex array and the index buffer
        // (which is the same for all) initially.
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer));
        m_Program.bind();


        // Paints every texture
        if (m_LayoutView)
        {
            // Computes the MVP matrix
            QMatrix4x4 mat_mvp;
            mat_mvp.setToIdentity();
            mat_mvp.ortho(0, width(), height(), 0, -1, 1);

            // Binds the vertex buffer of the current texture
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffers.at(0)));

            // Specifies the matrix and buffers within the shader program
            m_Program.setUniformValue("uni_mvp", mat_mvp);
            m_Program.setUniformValue("is_background", true);
            m_Program.enableAttributeArray(MV_VERTEX_ATTR);
            m_Program.enableAttributeArray(MV_COORD_ATTR);
            m_Program.setAttributeBuffer(MV_VERTEX_ATTR, GL_FLOAT, 0*sizeof(float), 2, 4*sizeof(float));
            m_Program.setAttributeBuffer(MV_COORD_ATTR,  GL_FLOAT, 2*sizeof(float), 2, 4*sizeof(float));

            // Binds the palette for the entire map
            glCheck(glActiveTexture(GL_TEXTURE1));
            glCheck(glBindTexture(GL_TEXTURE_2D, m_PalTextures.at(0)));

            // Draws the background
            glCheck(glActiveTexture(GL_TEXTURE0));
            glCheck(glBindTexture(GL_TEXTURE_2D, m_MapTextures.at(0)));
            glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));

            // Draws the foreground
            glCheck(glActiveTexture(GL_TEXTURE0));
            m_Program.setUniformValue("is_background", false);
            glCheck(glBindTexture(GL_TEXTURE_2D, m_MapTextures.at(1)));
            glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));
        }
        else
        {
            for (int i = 0; i < m_Maps.size(); i++)
            {
                if (i == 0)
                    m_Program.setUniformValue("is_connection", false);
                else if (i == 1)
                    m_Program.setUniformValue("is_connection", true);

                // Computes the MVP matrix with specific translation
                QMatrix4x4 mat_mvp;
                mat_mvp.setToIdentity();
                mat_mvp.ortho(0, width(), height(), 0, -1, 1);
                mat_mvp.translate(m_MapPositions.at(i).x(), m_MapPositions.at(i).y());

                // Binds the vertex buffer of the current texture
                glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffers.at(i)));

                // Specifies the matrix and buffers within the shader program
                m_Program.setUniformValue("uni_mvp", mat_mvp);
                m_Program.setUniformValue("is_background", true);
                m_Program.enableAttributeArray(MV_VERTEX_ATTR);
                m_Program.enableAttributeArray(MV_COORD_ATTR);
                m_Program.setAttributeBuffer(MV_VERTEX_ATTR, GL_FLOAT, 0*sizeof(float), 2, 4*sizeof(float));
                m_Program.setAttributeBuffer(MV_COORD_ATTR,  GL_FLOAT, 2*sizeof(float), 2, 4*sizeof(float));

                // Binds the palette for the entire map
                glCheck(glActiveTexture(GL_TEXTURE1));
                glCheck(glBindTexture(GL_TEXTURE_2D, m_PalTextures.at(i)));

                // Draws the background
                glCheck(glActiveTexture(GL_TEXTURE0));
                glCheck(glBindTexture(GL_TEXTURE_2D, m_MapTextures.at(i*2)));
                glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));

                m_Program.setUniformValue("is_background", false);

                // Draws the NPCs in between (but only the main map's ones)
                if (i == 0 && m_ShowSprites)
                {
                    QMatrix4x4 mat_npc;
                    for (int j = 0; j < m_NPCSprites.size(); j++)
                    {
                        float buf[16] = { 0, 0, 0, 0, 16, 0, 1, 0, 16, 16, 1, 1, 0, 16, 0, 1 };
                        glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_NPCBuffer));
                        glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*16, buf, GL_DYNAMIC_DRAW));

                        mat_npc.setToIdentity();
                        mat_mvp.ortho(0, width(), height(), 0, -1, 1);
                        mat_mvp.translate(m_NPCPositions.at(j).x(), m_NPCPositions.at(j).y());
                        m_Program.setUniformValue("uni_mvp", mat_npc);

                        glCheck(glActiveTexture(GL_TEXTURE1));
                        glCheck(glBindTexture(GL_TEXTURE_2D, m_NPCTextures.value(m_NPCSprites.at(j)).at(1)));
                        glCheck(glActiveTexture(GL_TEXTURE0));
                        glCheck(glBindTexture(GL_TEXTURE_2D, m_NPCTextures.value(m_NPCSprites.at(j)).at(0)));
                        glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));
                    }

                    // Resets to previous states
                    m_Program.setUniformValue("uni_mvp", mat_mvp);
                    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffers.at(i)));
                    glCheck(glActiveTexture(GL_TEXTURE1));
                    glCheck(glBindTexture(GL_TEXTURE_2D, m_PalTextures.at(i)));
                }


                // Draws the foreground
                glCheck(glActiveTexture(GL_TEXTURE0));
                glCheck(glBindTexture(GL_TEXTURE_2D, m_MapTextures.at(i*2+1)));
                glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));
            }

            m_Program.setUniformValue("is_connection", false);
        }
        if (m_MovementMode)
        {
            float buf[16] =
            {
                0,  0,  0, 0,
                16, 0,  1, 0,
                16, 16, 1, 1,
                0,  16, 0, 1
            };

            const float texWidth = 16;
            const float texHeight = 1024;
            glCheck(glActiveTexture(GL_TEXTURE0));
            glCheck(glBindTexture(GL_TEXTURE_2D, m_MoveTexture));
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_MoveBuffer));

            QMatrix4x4 mat_mvp;
            mat_mvp.setToIdentity();
            mat_mvp.ortho(0, width(), height(), 0, -1, 1);
            mat_mvp.translate(m_MapPositions.at(0).x(), m_MapPositions.at(0).y());
            m_RgbProgram.bind();
            m_RgbProgram.setUniformValue("uni_mvp", mat_mvp);
            m_RgbProgram.enableAttributeArray(MV_VERTEX_ATTR);
            m_RgbProgram.enableAttributeArray(MV_COORD_ATTR);
            m_RgbProgram.setAttributeBuffer(MV_VERTEX_ATTR, GL_FLOAT, 0*sizeof(float), 2, 4*sizeof(float));
            m_RgbProgram.setAttributeBuffer(MV_COORD_ATTR,  GL_FLOAT, 2*sizeof(float), 2, 4*sizeof(float));


            // Draws all the movement permissions
            const QSize mapSize = m_Maps.at(0)->header().size();
            const QList<MapBlock *> &blocks = m_Maps.at(0)->header().blocks();
            for (int i = 0; i < blocks.size(); i++)
            {
                float mapX = (i % mapSize.width()) * 16;
                float mapY = (i / mapSize.width()) * 16;
                float posX = 0;
                float posY = blocks.at(i)->permission * 16.0f + 0.5f;

                float glX = (posX == 0) ? 0 : posX / texWidth;
                float glY = (posY == 0) ? 0 : posY / texHeight;
                float glW = 16.0f / texWidth;
                float glH = 15.5f / texHeight;

                buf[0] = mapX;
                buf[1] = mapY;
                buf[2] = glX;
                buf[3] = glY;
                buf[4] = mapX + 16.0f;
                buf[5] = mapY;
                buf[6] = glX + glW;
                buf[7] = glY;
                buf[8] = mapX + 16.0f;
                buf[9] = mapY + 16.0f;
                buf[10] = glX + glW;
                buf[11] = glY + glH;
                buf[12] = mapX + 0.5f;
                buf[13] = mapY + 16.0f;
                buf[14] = glX;
                buf[15] = glY + glH;

                glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*16, buf, GL_DYNAMIC_DRAW));
                glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));
            }
        }

        // Show the cursor
        if (m_ShowCursor)
        {
            QMatrix4x4 mat_mvp;
            m_PmtProgram.bind();
            int mapWidth = m_MapSizes.at(0).width() / 16;
            int x = 0;
            int y = 0;
            int selectorWidth = 0;
            int selectorHeight = 0;

            if (m_SelectedBlocks.empty() && m_BlockView->selectedBlocks().empty())
            {
                // Computes the position on widget based on selected blocks
                int firstX = (m_FirstBlock % mapWidth);
                int firstY = (m_FirstBlock / mapWidth);
                int lastX = (m_LastBlock % mapWidth);
                int lastY = (m_LastBlock / mapWidth);

                x = firstX;
                y = firstY;

                selectorWidth = lastX - firstX;
                selectorHeight = lastY - firstY;

                if (lastX < firstX)
                {
                    x = lastX;
                    selectorWidth = firstX - x;
                }

                if (lastY < firstY)
                {
                    y = lastY;
                    selectorHeight = firstY - y;
                }
            }
            else if (m_BlockView->selectedBlocks().empty())
            {
                x = (m_HighlightedBlock % mapWidth);
                y = (m_HighlightedBlock / mapWidth);

                selectorWidth = m_SelectSize.width() - 1;
                selectorHeight = m_SelectSize.height() - 1;
            }
            else
            {
                // Computes the position on widget based on selected blocks
                int firstX = (m_BlockView->selectedBlocks().first() % 8);
                int firstY = (m_BlockView->selectedBlocks().first() / 8);
                int lastX = (m_BlockView->selectedBlocks().last() % 8);
                int lastY = (m_BlockView->selectedBlocks().last() / 8);

                selectorWidth = lastX - firstX;
                selectorHeight = lastY - firstY;

                x = (m_HighlightedBlock % mapWidth);
                y = (m_HighlightedBlock / mapWidth);
            }

            if (x + selectorWidth >= mapWidth)
                selectorWidth = mapWidth - x - 1;

            if (y + selectorHeight >= (m_MapSizes.at(0).height() / 16))
                selectorHeight = (m_MapSizes.at(0).height() / 16) - y - 1;

            // This should be made user-changable later
            x *= 16;
            y *= 16;
            selectorWidth *= 16;
            selectorHeight *= 16;

            QPoint mp = m_MapPositions.at(0);

            x += mp.x();
            y += mp.y();

            // Defines the uniform vertex attributes
            float vertRect[8]
            {
                0.5f,                0.5f,
                selectorWidth+15.5f, 0.5f,
                selectorWidth+15.5f, selectorHeight+15.5f,
                0.5f,                selectorHeight+15.5f
            };

            unsigned rectBuffer = 0;
            glCheck(glGenBuffers(1, &rectBuffer));
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, rectBuffer));

            mat_mvp.setToIdentity();
            mat_mvp.ortho(0, width(), height(), 0, -1, 1);
            mat_mvp.translate(x, y);

            // Modifies program states
            m_PmtProgram.enableAttributeArray(MV_VERTEX_ATTR);
            m_PmtProgram.setAttributeBuffer(MV_VERTEX_ATTR, GL_FLOAT, 0*sizeof(float), 2, 2*sizeof(float));
            m_PmtProgram.setUniformValue("uni_color", m_CursorColor);
            m_PmtProgram.setUniformValue("uni_mvp", mat_mvp);

            // Render current selected block rect
            //glCheck(glBindBuffer(GL_ARRAY_BUFFER, rectBuffer));
            glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*8, vertRect, GL_STATIC_DRAW));
            glCheck(glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, NULL));
            glCheck(glDeleteBuffers(1, &rectBuffer));
        }
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Helper
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    inline void flipVertically(UInt8 *dest, UInt8 *pixels)
    {
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
                dest[i + j * 8] = pixels[i + (8-1-j) * 8];
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Helper
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    inline void flipHorizontally(UInt8 *dest, UInt8 *pixels)
    {
        for (int i = 0; i < 64; i++)
            dest[i] = pixels[i - 2*(i%8) + 8-1];
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Helper
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    inline void extractTile(const QByteArray &img, Tile &tile)
    {
        unsigned char buffer[64];
        int x = (tile.tile % 16) * 8;
        int y = (tile.tile / 16) * 8;
        int pos = 0;

        for (int y2 = 0; y2 < 8; y2++)
            for (int x2 = 0; x2 < 8; x2++)
                pixelBuffer[pos++] = (UInt8)((img.at((x+x2) + (y+y2) * 128)) + (tile.palette * 16));

        if (tile.flipX)
        {
            flipHorizontally(buffer, pixelBuffer);
            if (tile.flipY)
                flipVertically(pixelBuffer, buffer);
            else
                memcpy(pixelBuffer, buffer, 64);
        }
        else if (tile.flipY)
        {
            flipVertically(buffer, pixelBuffer);
            memcpy(pixelBuffer, buffer, 64);
        }
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Helper
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    inline void extractBlock(UInt8 *pixels, UInt16 block)
    {
        int x = (block % 8) * 16;
        int y = (block / 8) * 16;
        int pos = 0;

        for (int y2 = 0; y2 < 16; y2++)
            for (int x2 = 0; x2 < 16; x2++)
                blockBuffer[pos++] = pixels[(x+x2) + (y+y2) * 128];
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Setter
    // Contributors:   Diegoisawesome
    // Last edit by:   Diegoisawesome
    // Date of edit:   10/24/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::setCurrentTool(AMEMapView::Tool tool)
    {
        m_CurrentTool = tool;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Diegoisawesome
    // Last edit by:   Diegoisawesome
    // Date of edit:   10/24/2016
    //
    ///////////////////////////////////////////////////////////
    AMEMapView::Tool AMEMapView::getCurrentTool(Qt::MouseButtons buttons)
    {
        if (m_CurrentTool != AMEMapView::Tool::None)
            return m_CurrentTool;
        if (buttons & Qt::LeftButton)
            return AMEMapView::Tool::Draw;
        else if (buttons & Qt::RightButton)
            return AMEMapView::Tool::Select;
        else if (buttons & Qt::MiddleButton)
        {
            if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier)
                return AMEMapView::Tool::FillAll;
            else
                return AMEMapView::Tool::Fill;
        }
        return AMEMapView::Tool::None;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Virtual
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   8/25/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::mousePressEvent(QMouseEvent *event)
    {
        int mouseX = event->pos().x();
        int mouseY = event->pos().y();
        QPoint mp = m_MapPositions.at(0);
        QSize mz = m_MapSizes.at(0);

        if (mouseX < mp.x()              || mouseY < mp.y()            ||
            mouseX > mp.x() + mz.width() || mouseY > mp.y() + mz.height())
        {
            return;
        }

        // Seems to be redundant?
        //if (mouseX >= width() || mouseY >= height())
        //    return;

        mouseX -= mp.x();
        mouseY -= mp.y();

        Map *mm = m_Maps[0];
        int mapWidth = mm->header().size().width();

        AMEMapView::Tool currentTool = getCurrentTool(event->buttons());

        if (currentTool == AMEMapView::Tool::Draw)
        {
            m_CursorColor = Qt::GlobalColor::red;
            QVector<UInt16> selected;
            int selectionWidth = 0;
            int selectionHeight = 0;
            if (m_BlockView->selectedBlocks().empty())
            {
                selected = m_SelectedBlocks;
                selectionWidth = m_SelectSize.width();
                selectionHeight = m_SelectSize.height();
            }
            else
            {
                selected = m_BlockView->selectedBlocks();
                selectionWidth = (selected.last() % 8) - (selected.first() % 8) + 1;
                selectionHeight = (selected.last() / 8) - (selected.first() / 8) + 1;
            }

            // Fetches relevant data
            UInt32 bg = m_MapTextures[0];
            UInt32 fg = m_MapTextures[1];

            // Determines the tile-number
            int mapBlockIndex = ((mouseY/16)*mapWidth) + (mouseX/16);

            // Sets the block in the data
            for (int i = 0; i < selectionHeight; i++)
            {
                for (int j = 0; j < selectionWidth; j++)
                {
                    if ((mapBlockIndex + j + (i * mapWidth)) >= (mapWidth * mm->header().size().height()))
                        continue;
                    mm->header().blocks()[mapBlockIndex + j + (i * mapWidth)]->block = (UInt16)selected[j + (i * selectionWidth)];

                    int currBlockIndex = selected[j + (i * selectionWidth)];

                    // Sets the block in the actual image (BG & FG)
                    if (currBlockIndex >= m_PrimaryBlockCount)
                        extractBlock(m_SecondaryBackground, currBlockIndex - m_PrimaryBlockCount);
                    else
                        extractBlock(m_PrimaryBackground, currBlockIndex);
                    glBindTexture(GL_TEXTURE_2D, bg);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, ((mouseX / 16) + j) * 16, ((mouseY / 16) + i) * 16, 16, 16, GL_RED, GL_UNSIGNED_BYTE, blockBuffer);

                    if (currBlockIndex >= m_PrimaryBlockCount)
                        extractBlock(m_SecondaryForeground, currBlockIndex - m_PrimaryBlockCount);
                    else
                        extractBlock(m_PrimaryForeground, currBlockIndex);
                    glBindTexture(GL_TEXTURE_2D, fg);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, ((mouseX / 16) + j) * 16, ((mouseY / 16) + i) * 16, 16, 16, GL_RED, GL_UNSIGNED_BYTE, blockBuffer);
                }
            }
        }
        else if (currentTool == AMEMapView::Tool::Select)
        {
            m_CursorColor = Qt::GlobalColor::yellow;

            // Determines the moused-over block number
            m_FirstBlock = (mouseX/16) + ((mouseY/16)*mapWidth);
            m_LastBlock = m_FirstBlock;
            m_SelectedBlocks.clear();
            m_BlockView->deselectBlocks();
            m_BlockView->repaint();
        }
        repaint();
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Virtual
    // Contributors:   Diegoisawesome
    // Last edit by:   Diegoisawesome
    // Date of edit:   10/6/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::mouseReleaseEvent(QMouseEvent *event)
    {
        m_CursorColor = Qt::GlobalColor::green;

        AMEMapView::Tool currentTool = getCurrentTool(event->button());

        if (currentTool == AMEMapView::Tool::Select)
        {
            Map *mm = m_Maps[0];

            // Determines the tile-number
            int mapWidth = mm->header().size().width();

            int selectionWidth = (m_LastBlock % mapWidth) - (m_FirstBlock % mapWidth);
            if (selectionWidth < 0)
            {
                selectionWidth = -selectionWidth;
                m_FirstBlock -= selectionWidth;
                m_LastBlock += selectionWidth;
            }
            selectionWidth++;

            int selectionHeight = (m_LastBlock / mapWidth) - (m_FirstBlock / mapWidth);
            if (selectionHeight < 0)
            {
                selectionHeight = -selectionHeight;
                m_FirstBlock -= selectionHeight * mapWidth;
                m_LastBlock += selectionHeight * mapWidth;
            }
            selectionHeight++;

            m_SelectSize = QSize(selectionWidth, selectionHeight);

            m_SelectedBlocks.clear();

            for (int i = 0; i < selectionHeight; i++)
            {
                for (int j = 0; j < selectionWidth; j++)
                {
                    m_SelectedBlocks.push_back(mm->header().blocks()[m_FirstBlock + j + (i * mm->header().size().width())]->block);
                }
            }

            if (m_SelectedBlocks.length() == 1)
            {
                m_BlockView->selectBlock(m_SelectedBlocks[0]);
            }
        }
        repaint();
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Virtual
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   9/26/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::mouseMoveEvent(QMouseEvent *event)
    {
        int mouseX = event->pos().x();
        int mouseY = event->pos().y();
        bool needsRepaint = false;

        QPoint mp = m_MapPositions.at(0);
        QSize mz = m_MapSizes.at(0);

        mouseX -= mp.x();
        mouseY -= mp.y();

        AMEMapView::Tool currentTool = getCurrentTool(event->buttons());

        if (currentTool == AMEMapView::Tool::Select)
        {
            if (mouseX < 0)
                mouseX = 0;
            else if (mouseX >= mz.width())
                mouseX = mz.width() - 1;

            if (mouseY < 0)
                mouseY = 0;
            else if (mouseY >= mz.height())
                mouseY = mz.height() - 1;

            if (m_LastBlock != (mouseX/16) + ((mouseY/16)*(mz.width()/16)))
            {
                needsRepaint = true;
                m_LastBlock = (mouseX/16) + ((mouseY/16)*(mz.width()/16));
            }
        }
        else
        {
            if (mouseX < 0           || mouseY < 0            ||
                mouseX >= mz.width() || mouseY >= mz.height())
            {
                m_ShowCursor = false;
                repaint();
                return;
            }

            if (currentTool == AMEMapView::Tool::Draw)
            {
                // Determines the block number
                mousePressEvent(event);
            }
        }

        if (m_ShowCursor != true)
        {
            needsRepaint = true;
            m_ShowCursor = true;
        }

        if (m_HighlightedBlock != (mouseX/16) + ((mouseY/16)*(mz.width()/16)))
        {
            needsRepaint = true;
            m_HighlightedBlock = (mouseX/16) + ((mouseY/16)*(mz.width()/16));
        }

        if (needsRepaint)
            repaint();
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Virtual
    // Contributors:   Diegoisawesome
    // Last edit by:   Diegoisawesome
    // Date of edit:   10/7/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::leaveEvent(QEvent *event)
    {
        Q_UNUSED(event);
        m_ShowCursor = false;

        repaint();
    }

    ///////////////////////////////////////////////////////////
    // Function type:  I/O
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    bool AMEMapView::setMap(const qboy::Rom &rom, Map *mainMap)
    {
        Q_UNUSED(rom);
        const QSize mainSize = mainMap->header().size();
        const QList<Connection *> connex = mainMap->connections().connections();
        m_Maps.push_back(mainMap);
        m_LayoutView = false;


        // Retrieves every connected map
        for (int i = 0; i < connex.size(); i++)
        {
            Connection *current = connex[i];
            Map *map = dat_MapBankTable->banks().at(current->bank)->maps().at(current->map);
            m_Maps.push_back(map);
        }

        m_MapSizes.push_back(QSize(mainSize.width()*16, mainSize.height()*16));
        m_MapPositions.push_back(QPoint(0, 0));
        m_WidgetSize = QSize(m_MapSizes.at(0));


        // Calculates the positions for the maps
        int biggestLeftMap = 0; // width of the biggest left-connected map
        int biggestTopMap = 0;  // height of the biggest top-connected map
        const int defaultRowCount = 8;
        for (int i = 0; i < connex.size(); i++)
        {
            Connection *current = connex[i];
            Map *map = m_Maps[i+1];
            QPoint translation;
            Int32 signedOff = (Int32)(current->offset)*16;

            // Determines the offset by direction
            QSize mapSize = QSize(map->header().size().width()*16, map->header().size().height()*16);
            if (current->direction == DIR_Left)
            {
                int rowCount = defaultRowCount;
                if (map->header().size().width() < rowCount)
                    rowCount = map->header().size().width();

                m_ConnectParts.push_back(QPoint(mapSize.width()-(16*rowCount), 0));
                mapSize.setWidth(16*rowCount);
                translation.setX(-mapSize.width());
                translation.setY(signedOff);
                m_WidgetSize.rwidth() += mapSize.width();
                m_MaxRows.push_back(rowCount);

                if (mapSize.width() > biggestLeftMap)
                    biggestLeftMap = mapSize.width();
            }
            else if (current->direction == DIR_Right)
            {
                int rowCount = defaultRowCount;
                if (map->header().size().width() < rowCount)
                    rowCount = map->header().size().width();

                m_ConnectParts.push_back(QPoint(0, 0));
                mapSize.setWidth(16*rowCount);
                translation.setX(mainSize.width()*16);
                translation.setY(signedOff);
                m_WidgetSize.rwidth() += mapSize.width();
                m_MaxRows.push_back(rowCount);
            }
            else if (current->direction == DIR_Up)
            {
                int rowCount = defaultRowCount;
                if (map->header().size().width() < rowCount)
                    rowCount = map->header().size().width();

                m_ConnectParts.push_back(QPoint(0, mapSize.height()-(16*rowCount)));
                mapSize.setHeight(16*rowCount);
                translation.setX(signedOff);
                translation.setY(-mapSize.height());
                m_WidgetSize.rheight() += mapSize.height();
                m_MaxRows.push_back(rowCount);

                if (mapSize.height() > biggestTopMap)
                    biggestTopMap = mapSize.height();
            }
            else if (current->direction == DIR_Down)
            {
                int rowCount = defaultRowCount;
                if (map->header().size().width() < rowCount)
                    rowCount = map->header().size().width();

                m_ConnectParts.push_back(QPoint(0, 0));
                mapSize.setHeight(16*rowCount);
                translation.setX(signedOff);
                translation.setY(mainSize.height()*16);
                m_WidgetSize.rheight() += mapSize.height();
                m_MaxRows.push_back(rowCount);
            }

            m_MapPositions.push_back(translation);
            m_MapSizes.push_back(mapSize);
        }

        // Adds the biggest left and top offsets to all positions
        // in order to not cause rendering fails on the widget
        for (int i = 0; i < m_MapPositions.size(); i++)
        {
            m_MapPositions[i].rx() += biggestLeftMap;
            m_MapPositions[i].ry() += biggestTopMap;
        }


        // Determines the block count for each game
        int blockCountPrimary;
        int blockCountSecondary;
        if (CONFIG(RomType) == RT_FRLG)
        {
            blockCountPrimary = 0x280;
            blockCountSecondary = 0x180;
        }
        else
        {
            blockCountPrimary = 0x200;
            blockCountSecondary = 0x200;
        }

        m_PrimaryBlockCount = blockCountPrimary;
        m_SecondaryBlockCount = blockCountSecondary;


        // Attempts to load all the blocksets
        QList<UInt8 *> blocksetBack;
        QList<UInt8 *> blocksetFore;
        for (int i = 0; i < m_Maps.size(); i++)
        {
            Tileset *primary = m_Maps[i]->header().primary();
            Tileset *secondary = m_Maps[i]->header().secondary();
            QVector<qboy::GLColor> palettes;

            // Retrieves the palettes, combines them, removes bg color
            for (int j = 0; j < primary->palettes().size(); j++)
            {
                palettes.append(primary->palettes().at(j)->rawGL());
                palettes[j * 16].a = 0.0f;
            }
            for (int j = 0; j < secondary->palettes().size(); j++)
            {
                palettes.append(secondary->palettes().at(j)->rawGL());
                palettes[(primary->palettes().size() * 16) + (j * 16)].a = 0.0f;
            }

            while (palettes.size() < 256) // align pal for OpenGL
                palettes.push_back({ 0.f, 0.f, 0.f, 0.f });

            m_Palettes.push_back(palettes);


            // Retrieves the raw pixel data of the tilesets
            const QByteArray &priRaw = primary->image()->raw();
            const QByteArray &secRaw = secondary->image()->raw();
            const int secRawMax = (128/8 * secondary->image()->size().height()/8);

            // Creates two buffers for the blockset pixels
            Int32 tilesetHeight1 = blockCountPrimary / 8 * 16;
            Int32 tilesetHeight2 = blockCountSecondary / 8 * 16;
            UInt8 *background1 = new UInt8[128 * tilesetHeight1];
            UInt8 *foreground1 = new UInt8[128 * tilesetHeight1];
            UInt8 *background2 = new UInt8[128 * tilesetHeight2];
            UInt8 *foreground2 = new UInt8[128 * tilesetHeight2];

            // Copy the tileset sizes for our main map
            if (i == 0)
            {
                m_PrimarySetSize = QSize(128, tilesetHeight1);
                m_SecondarySetSize = QSize(128, tilesetHeight2);
            }


            // Parses all the primary block data
            for (int j = 0; j < primary->blocks().size(); j++)
            {
                Block *curBlock = primary->blocks()[j];
                Int32 blockX = (j % 8) * 16;
                Int32 blockY = (j / 8) * 16;

                /* BACKGROUND */
                for (int k = 0; k < 4; k++)
                {
                    Tile tile = curBlock->tiles[k];
                    Int32 subX = ((k % 2) * 8) + blockX;
                    Int32 subY = ((k / 2) * 8) + blockY;

                    if (tile.tile >= blockCountPrimary)
                    {
                        tile.tile -= blockCountPrimary;

                        if (secRawMax > tile.tile)
                            extractTile(secRaw, tile);
                        else
                            memset(pixelBuffer, 0, 64);
                    }
                    else
                    {
                        extractTile(priRaw, tile);
                    }

                    int pos = 0;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++)
                            background1[(x+subX) + (y+subY) * 128] = pixelBuffer[pos++];
                }

                /* FOREGROUND */
                for (int k = 0; k < 4; k++)
                {
                    Tile tile = curBlock->tiles[k+4];
                    Int32 subX = ((k % 2) * 8) + blockX;
                    Int32 subY = ((k / 2) * 8) + blockY;;

                    if (tile.tile >= blockCountPrimary)
                    {
                        tile.tile -= blockCountPrimary;

                        if (secRawMax > tile.tile)
                            extractTile(secRaw, tile);
                        else
                            memset(pixelBuffer, 0, 64);
                    }
                    else
                    {
                        extractTile(priRaw, tile);
                    }

                    int pos = 0;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++)
                            foreground1[(x+subX) + (y+subY) * 128] = pixelBuffer[pos++];
                }
            }

            // Parses all the secondary block data
            for (int j = 0; j < secondary->blocks().size(); j++)
            {
                Block *curBlock = secondary->blocks()[j];
                Int32 blockX = (j % 8) * 16;
                Int32 blockY = (j / 8) * 16;

                /* BACKGROUND */
                for (int k = 0; k < 4; k++)
                {
                    Tile tile = curBlock->tiles[k];
                    Int32 subX = ((k % 2) * 8) + blockX;
                    Int32 subY = ((k / 2) * 8) + blockY;

                    if (tile.tile >= blockCountPrimary)
                    {
                        tile.tile -= blockCountPrimary;

                        if (secRawMax > tile.tile)
                            extractTile(secRaw, tile);
                        else
                            memset(pixelBuffer, 0, 64);
                    }
                    else
                    {
                        extractTile(priRaw, tile);
                    }

                    int pos = 0;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++)
                            background2[(x+subX) + (y+subY) * 128] = pixelBuffer[pos++];
                }

                /* FOREGROUND */
                for (int k = 0; k < 4; k++)
                {
                    Tile tile = curBlock->tiles[k+4];
                    Int32 subX = ((k % 2) * 8) + blockX;
                    Int32 subY = ((k / 2) * 8) + blockY;;

                    if (tile.tile >= blockCountPrimary)
                    {
                        tile.tile -= blockCountPrimary;

                        if (secRawMax > tile.tile)
                            extractTile(secRaw, tile);
                        else
                            memset(pixelBuffer, 0, 64);
                    }
                    else
                    {
                        extractTile(priRaw, tile);
                    }

                    int pos = 0;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++)
                            foreground2[(x+subX) + (y+subY) * 128] = pixelBuffer[pos++];
                }
            }


            // Adds the new blocksets
            blocksetBack.push_back(background1);
            blocksetBack.push_back(background2);
            blocksetFore.push_back(foreground1);
            blocksetFore.push_back(foreground2);
        }


        // Creates the image for the main map
        for (int i = 0; i < 1; i++)
        {
            MapHeader &header = m_Maps[i]->header();
            QSize mapSize = header.size();

            // Creates a new pixel buffer for the map
            UInt8 *backMapBuffer = new UInt8[mapSize.width()*16 * mapSize.height()*16];
            UInt8 *foreMapBuffer = new UInt8[mapSize.width()*16 * mapSize.height()*16];
            UInt8 *primaryBg = blocksetBack.at(i*2);
            UInt8 *secondaryBg = blocksetBack.at(i*2+1);
            UInt8 *primaryFg = blocksetFore.at(i*2);
            UInt8 *secondaryFg = blocksetFore.at(i*2+1);

            // Iterates through every map block and writes it to the map buffer
            for (int j = 0; j < header.blocks().size(); j++)
            {
                MapBlock block = *header.blocks().at(j);
                Int32 mapX = (j % mapSize.width()) * 16;
                Int32 mapY = (j / mapSize.width()) * 16;

                if (block.block >= blockCountPrimary)
                    extractBlock(secondaryBg, block.block - blockCountPrimary);
                else
                    extractBlock(primaryBg, block.block);

                int pos = 0;
                for (int y = 0; y < 16; y++)
                    for (int x = 0; x < 16; x++)
                        backMapBuffer[(x+mapX) + (y+mapY) * mapSize.width()*16] = blockBuffer[pos++];

                if (block.block >= blockCountPrimary)
                    extractBlock(secondaryFg, block.block - blockCountPrimary);
                else
                    extractBlock(primaryFg, block.block);

                pos = 0;
                for (int y = 0; y < 16; y++)
                    for (int x = 0; x < 16; x++)
                        foreMapBuffer[(x+mapX) + (y+mapY) * mapSize.width()*16] = blockBuffer[pos++];
            }

            // Appends the pixel buffers
            m_BackPixelBuffers.push_back(backMapBuffer);
            m_ForePixelBuffers.push_back(foreMapBuffer);
        }

        // Renders all the connected maps
        for (int n = 0; n < m_ConnectParts.size(); n++)
        {
            MapHeader &header = m_Maps[n+1]->header();

            // Determines the start block
            const QPoint pos = m_ConnectParts.at(n);
            const QSize mapSize = m_Maps.at(n+1)->header().size();
            const QSize absSize = m_MapSizes.at(n+1);
            int start = (pos.x() / 16) + ((pos.y() / 16) * mapSize.width());

            // Creates the pixel buffers for the connected map
            UInt8 *backMapBuffer = new UInt8[absSize.width()*absSize.height()];
            UInt8 *foreMapBuffer = new UInt8[absSize.width()*absSize.height()];
            UInt8 *primaryBg = blocksetBack.at((n+1)*2);
            UInt8 *secondaryBg = blocksetBack.at((n+1)*2+1);
            UInt8 *primaryFg = blocksetFore.at((n+1)*2);
            UInt8 *secondaryFg = blocksetFore.at((n+1)*2+1);
            DirectionType dir = m_Maps.at(0)->connections().connections().at(n)->direction;
            Int32 rowCount = m_MaxRows.at(n);


            // Iterates through every visible block
            if (dir == DIR_Left || dir == DIR_Right)
            {
                for (int m = 0; m < (absSize.width()/16)*(absSize.height()/16); m++)
                {
                    int processed = (m/rowCount)*mapSize.width();
                    int blockNumber = start + processed + (m%rowCount);

                    MapBlock block = *header.blocks().at(blockNumber);
                    Int32 mapX = (m % (absSize.width()/16)) * 16;
                    Int32 mapY = (m / (absSize.width()/16)) * 16;

                    if (block.block >= blockCountPrimary)
                        extractBlock(secondaryBg, block.block - blockCountPrimary);
                    else
                        extractBlock(primaryBg, block.block);

                    int pos = 0;
                    for (int y = 0; y < 16; y++)
                        for (int x = 0; x < 16; x++)
                            backMapBuffer[(x+mapX) + (y+mapY) * absSize.width()] = blockBuffer[pos++];

                    if (block.block >= blockCountPrimary)
                        extractBlock(secondaryFg, block.block - blockCountPrimary);
                    else
                        extractBlock(primaryFg, block.block);

                    pos = 0;
                    for (int y = 0; y < 16; y++)
                        for (int x = 0; x < 16; x++)
                            foreMapBuffer[(x+mapX) + (y+mapY) * absSize.width()] = blockBuffer[pos++];
                }
            }
            else if (dir == DIR_Down || dir == DIR_Up)
            {
                for (int m = 0; m < (absSize.width()/16)*(absSize.height()/16); m++)
                {
                    int blockNumber = start + m;

                    MapBlock block = *header.blocks().at(blockNumber);
                    Int32 mapX = (m % mapSize.width()) * 16;
                    Int32 mapY = (m / mapSize.width()) * 16;

                    if (block.block >= blockCountPrimary)
                        extractBlock(secondaryBg, block.block - blockCountPrimary);
                    else
                        extractBlock(primaryBg, block.block);

                    int pos = 0;
                    for (int y = 0; y < 16; y++)
                        for (int x = 0; x < 16; x++)
                            backMapBuffer[(x+mapX) + (y+mapY) * mapSize.width()*16] = blockBuffer[pos++];

                    if (block.block >= blockCountPrimary)
                        extractBlock(secondaryFg, block.block - blockCountPrimary);
                    else
                        extractBlock(primaryFg, block.block);

                    pos = 0;
                    for (int y = 0; y < 16; y++)
                        for (int x = 0; x < 16; x++)
                            foreMapBuffer[(x+mapX) + (y+mapY) * mapSize.width()*16] = blockBuffer[pos++];
                }
            }

            // Appends the pixel buffers
            m_BackPixelBuffers.push_back(backMapBuffer);
            m_ForePixelBuffers.push_back(foreMapBuffer);
        }

        // Deletes all unnecessary blocksets
        for (int i = 0; i < blocksetBack.size(); i++)
        {
            if (i == 0)
            {
                m_PrimaryBackground = blocksetBack[0];
                m_PrimaryForeground = blocksetFore[0];
            }
            else if (i == 1)
            {
                m_SecondaryBackground = blocksetBack[1];
                m_SecondaryForeground = blocksetFore[1];
            }
            else
            {
                delete blocksetBack[i];
                delete blocksetFore[i];
            }

        }

        return true;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  I/O
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    bool AMEMapView::setLayout(MapHeader &mainMap)
    {
        const QSize mainSize = mainMap.size();
        m_Maps.push_back(NULL);
        m_MapSizes.push_back(QSize(mainSize.width()*16, mainSize.height()*16));
        m_MapPositions.push_back(QPoint(0, 0));
        m_WidgetSize = QSize(m_MapSizes.at(0));
        m_Header = mainMap;
        m_LayoutView = true;


        // Determines the block count for each game
        int blockCountPrimary;
        int blockCountSecondary;
        if (CONFIG(RomType) == RT_FRLG)
        {
            blockCountPrimary = 0x280;
            blockCountSecondary = 0x180;
        }
        else
        {
            blockCountPrimary = 0x200;
            blockCountSecondary = 0x200;
        }

        m_PrimaryBlockCount = blockCountPrimary;
        m_SecondaryBlockCount = blockCountSecondary;


        // Attempts to load all the blocksets
        QList<UInt8 *> blocksetBack;
        QList<UInt8 *> blocksetFore;
        for (int i = 0; i < 1; i++)
        {
            Tileset *primary = mainMap.primary();
            Tileset *secondary = mainMap.secondary();
            QVector<qboy::GLColor> palettes;

            // Retrieves the palettes, combines them, removes bg color
            for (int j = 0; j < primary->palettes().size(); j++)
            {
                palettes.append(primary->palettes().at(j)->rawGL());
                palettes[j * 16].a = 0.0f;
            }
            for (int j = 0; j < secondary->palettes().size(); j++)
            {
                palettes.append(secondary->palettes().at(j)->rawGL());
                palettes[(primary->palettes().size() * 16) + (j * 16)].a = 0.0f;
            }

            while (palettes.size() < 256) // align pal for OpenGL
                palettes.push_back({ 0.f, 0.f, 0.f, 0.f });

            m_Palettes.push_back(palettes);


            // Retrieves the raw pixel data of the tilesets
            const QByteArray &priRaw = primary->image()->raw();
            const QByteArray &secRaw = secondary->image()->raw();
            const int secRawMax = (128/8 * secondary->image()->size().height()/8);

            // Creates two buffers for the blockset pixels
            Int32 tilesetHeight1 = blockCountPrimary / 8 * 16;
            Int32 tilesetHeight2 = blockCountSecondary / 8 * 16;
            UInt8 *background1 = new UInt8[128 * tilesetHeight1];
            UInt8 *foreground1 = new UInt8[128 * tilesetHeight1];
            UInt8 *background2 = new UInt8[128 * tilesetHeight2];
            UInt8 *foreground2 = new UInt8[128 * tilesetHeight2];

            // Copy the tileset sizes for our main map
            if (i == 0)
            {
                m_PrimarySetSize = QSize(128, tilesetHeight1);
                m_SecondarySetSize = QSize(128, tilesetHeight2);
            }

            // Parses all the primary block data
            for (int j = 0; j < primary->blocks().size(); j++)
            {
                Block *curBlock = primary->blocks()[j];
                Int32 blockX = (j % 8) * 16;
                Int32 blockY = (j / 8) * 16;

                /* BACKGROUND */
                for (int k = 0; k < 4; k++)
                {
                    Tile tile = curBlock->tiles[k];
                    Int32 subX = ((k % 2) * 8) + blockX;
                    Int32 subY = ((k / 2) * 8) + blockY;

                    if (tile.tile >= blockCountPrimary)
                    {
                        tile.tile -= blockCountPrimary;

                        if (secRawMax > tile.tile)
                            extractTile(secRaw, tile);
                        else
                            memset(pixelBuffer, 0, 64);
                    }
                    else
                    {
                        extractTile(priRaw, tile);
                    }

                    int pos = 0;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++)
                            background1[(x+subX) + (y+subY) * 128] = pixelBuffer[pos++];
                }

                /* FOREGROUND */
                for (int k = 0; k < 4; k++)
                {
                    Tile tile = curBlock->tiles[k+4];
                    Int32 subX = ((k % 2) * 8) + blockX;
                    Int32 subY = ((k / 2) * 8) + blockY;;

                    if (tile.tile >= blockCountPrimary)
                    {
                        tile.tile -= blockCountPrimary;

                        if (secRawMax > tile.tile)
                            extractTile(secRaw, tile);
                        else
                            memset(pixelBuffer, 0, 64);
                    }
                    else
                    {
                        extractTile(priRaw, tile);
                    }

                    int pos = 0;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++)
                            foreground1[(x+subX) + (y+subY) * 128] = pixelBuffer[pos++];
                }
            }

            // Parses all the secondary block data
            for (int j = 0; j < secondary->blocks().size(); j++)
            {
                Block *curBlock = secondary->blocks()[j];
                Int32 blockX = (j % 8) * 16;
                Int32 blockY = (j / 8) * 16;

                /* BACKGROUND */
                for (int k = 0; k < 4; k++)
                {
                    Tile tile = curBlock->tiles[k];
                    Int32 subX = ((k % 2) * 8) + blockX;
                    Int32 subY = ((k / 2) * 8) + blockY;

                    if (tile.tile >= blockCountPrimary)
                    {
                        tile.tile -= blockCountPrimary;

                        if (secRawMax > tile.tile)
                            extractTile(secRaw, tile);
                        else
                            memset(pixelBuffer, 0, 64);
                    }
                    else
                    {
                        extractTile(priRaw, tile);
                    }

                    int pos = 0;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++)
                            background2[(x+subX) + (y+subY) * 128] = pixelBuffer[pos++];
                }

                /* FOREGROUND */
                for (int k = 0; k < 4; k++)
                {
                    Tile tile = curBlock->tiles[k+4];
                    Int32 subX = ((k % 2) * 8) + blockX;
                    Int32 subY = ((k / 2) * 8) + blockY;;

                    if (tile.tile >= blockCountPrimary)
                    {
                        tile.tile -= blockCountPrimary;

                        if (secRawMax > tile.tile)
                            extractTile(secRaw, tile);
                        else
                            memset(pixelBuffer, 0, 64);
                    }
                    else
                    {
                        extractTile(priRaw, tile);
                    }

                    int pos = 0;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++)
                            foreground2[(x+subX) + (y+subY) * 128] = pixelBuffer[pos++];
                }
            }


            // Adds the new blocksets
            blocksetBack.push_back(background1);
            blocksetBack.push_back(background2);
            blocksetFore.push_back(foreground1);
            blocksetFore.push_back(foreground2);
        }


        // Creates the images for the actual maps
        for (int i = 0; i < m_Maps.size(); i++)
        {
            MapHeader &header = m_Maps[i]->header();
            QSize mapSize = m_MapSizes[i];

            // Creates a new pixel buffer for the map
            UInt8 *backMapBuffer = new UInt8[mapSize.width() * mapSize.height()];
            UInt8 *foreMapBuffer = new UInt8[mapSize.width() * mapSize.height()];
            UInt8 *primaryBg = blocksetBack.at(i*2);
            UInt8 *secondaryBg = blocksetBack.at(i*2+1);
            UInt8 *primaryFg = blocksetFore.at(i*2);
            UInt8 *secondaryFg = blocksetFore.at(i*2+1);

            // Iterates through every map block and writes it to the map buffer
            for (int j = 0; j < header.blocks().size(); j++)
            {
                MapBlock block = *header.blocks().at(j);
                Int32 mapX = (j % mapSize.width()) * 16;
                Int32 mapY = (j / mapSize.width()) * 16;

                if (block.block >= blockCountPrimary)
                    extractBlock(secondaryBg, block.block - blockCountPrimary);
                else
                    extractBlock(primaryBg, block.block);

                int pos = 0;
                for (int y = 0; y < 16; y++)
                    for (int x = 0; x < 16; x++)
                        backMapBuffer[(x+mapX) + (y+mapY) * mapSize.width()*16] = blockBuffer[pos++];

                if (block.block >= blockCountPrimary)
                    extractBlock(secondaryFg, block.block - blockCountPrimary);
                else
                    extractBlock(primaryFg, block.block);

                pos = 0;
                for (int y = 0; y < 16; y++)
                    for (int x = 0; x < 16; x++)
                        foreMapBuffer[(x+mapX) + (y+mapY) * mapSize.width()*16] = blockBuffer[pos++];
            }

            // Appends the pixel buffers
            m_BackPixelBuffers.push_back(backMapBuffer);
            m_ForePixelBuffers.push_back(foreMapBuffer);
        }

        // Deletes all unnecessary blocksets
        for (int i = 0; i < blocksetBack.size(); i++)
        {
            if (i == 0)
            {
                m_PrimaryBackground = blocksetBack[0];
                m_PrimaryForeground = blocksetFore[0];
            }
            else if (i == 1)
            {
                m_SecondaryBackground = blocksetBack[1];
                m_SecondaryForeground = blocksetFore[1];
            }
            else
            {
                delete blocksetBack[i];
                delete blocksetFore[i];
            }

        }
        return true;
    }


    ///////////////////////////////////////////////////////////
    // Function type:  Setter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/21/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::setEntities(const QList<Npc *> &npcs)
    {
        makeCurrent();


        foreach (Npc *npc, npcs)
        {
            m_NPCPositions.push_back(QPoint(npc->positionX*16, npc->positionY*16));
            m_NPCSprites.push_back(npc->imageID);

            // Checks whether the NPC image is already buffered
            if (!m_NPCTextures.contains(npc->imageID))
            {
                qboy::Image *img = dat_OverworldTable->images().at(npc->imageID);
                qboy::Palette *pal = dat_OverworldTable->palettes().at(npc->imageID);

                // Allocates a new OpenGL texture for the image
                unsigned imageTex = 0;
                glCheck(glGenTextures(1, &imageTex));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, img->size().width(), img->size().height(), 0, GL_RED, GL_BYTE, img->raw().data()));

                // Allocates a new OpenGL texture for the palette
                unsigned palTex = 0;
                glCheck(glGenTextures(1, &palTex));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 1, 0, GL_RGBA, GL_FLOAT, pal->rawGL().data()));

                // Stores stuff
                m_NPCTextures.insert(npc->imageID, { imageTex, palTex });
            }
        }

        glCheck(glGenBuffers(1, &m_NPCBuffer));


        doneCurrent();
    }


    ///////////////////////////////////////////////////////////
    // Function type:  I/O
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/17/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::makeGL()
    {
        makeCurrent();
        m_VAO.bind();
        m_IsInit = true;

        // Creates, binds and buffers the static index buffer
        const unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
        glCheck(glGenBuffers(1, &m_IndexBuffer));
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer));
        glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned)*6, indices, GL_STATIC_READ));

        // Creates vertex buffers (we only need 1 for 2 layers)
        for (int i = 0; i < m_Maps.size(); i++)
        {
            // Creates a constant vertex buffer
            float wid = (float)m_MapSizes.at(i).width();
            float hei = (float)m_MapSizes.at(i).height();
            float buffer[16] =
            {
            //  X    Y       U   V
                0,   0,      0,  0,
                wid, 0,      1,  0,
                wid, hei,    1,  1,
                0,   hei,    0,  1
            };

            // Buffers the vertices
            unsigned vbuffer = 0;
            glCheck(glGenBuffers(1, &vbuffer));
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, vbuffer));
            glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(float)*16, buffer, GL_DYNAMIC_DRAW));
            m_VertexBuffers.push_back(vbuffer);


            // Creates foreground and background textures
            unsigned texFore = 0, texBack = 0;
            glCheck(glGenTextures(1, &texFore));
            glCheck(glGenTextures(1, &texBack));
            glCheck(glBindTexture(GL_TEXTURE_2D, texBack));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (int)wid, (int)hei, 0, GL_RED, GL_UNSIGNED_BYTE, m_BackPixelBuffers[i]));
            glCheck(glBindTexture(GL_TEXTURE_2D, texFore));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (int)wid, (int)hei, 0, GL_RED, GL_UNSIGNED_BYTE, m_ForePixelBuffers[i]));
            m_MapTextures.push_back(texBack);
            m_MapTextures.push_back(texFore);

            // Creates one palette texture for both layers
            unsigned texPal = 0;
            glCheck(glGenTextures(1, &texPal));
            glCheck(glBindTexture(GL_TEXTURE_2D, texPal));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_FLOAT, m_Palettes.at(i).data()));
            m_PalTextures.push_back(texPal);
        }

        // Sets the minimum size
        setMinimumSize(m_WidgetSize);
        doneCurrent();
    }


    ///////////////////////////////////////////////////////////
    // Function type:  I/O
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/20/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::freeGL()
    {
        makeCurrent();


        foreach (UInt32 id, m_MapTextures)
            glCheck(glDeleteTextures(1, &id));
        foreach (UInt32 id, m_PalTextures)
            glCheck(glDeleteTextures(1, &id));
        foreach (UInt32 id, m_VertexBuffers)
            glCheck(glDeleteBuffers(1, &id));
        foreach (UInt8 *v, m_BackPixelBuffers)
            delete[] v;
        foreach (UInt8 *v, m_ForePixelBuffers)
            delete[] v;

        glCheck(glDeleteBuffers(1, &m_IndexBuffer));
        delete[] m_PrimaryForeground;
        delete[] m_PrimaryBackground;
        delete[] m_SecondaryForeground;
        delete[] m_SecondaryBackground;

        m_Maps.clear();
        m_MapSizes.clear();
        m_MapPositions.clear();
        m_Palettes.clear();
        m_BackPixelBuffers.clear();
        m_ForePixelBuffers.clear();
        m_VertexBuffers.clear();
        m_PalTextures.clear();
        m_MapTextures.clear();
        m_ConnectParts.clear();

        doneCurrent();
        repaint();
    }


    ///////////////////////////////////////////////////////////
    // Function type:  Setter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   7/6/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::setMovementMode(bool isInMovementMode)
    {
        m_MovementMode = isInMovementMode;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Setter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   7/6/2016
    //
    ///////////////////////////////////////////////////////////
    void AMEMapView::setBlockView(AMEBlockView *view)
    {
        m_BlockView = view;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   7/6/2016
    //
    ///////////////////////////////////////////////////////////
    bool AMEMapView::movementMode() const
    {
        return m_MovementMode;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/18/2016
    //
    ///////////////////////////////////////////////////////////
    QPoint AMEMapView::mainPos()
    {
        return m_MapPositions[0];
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/18/2016
    //
    ///////////////////////////////////////////////////////////
    Map *AMEMapView::mainMap()
    {
        return m_Maps[0];
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/18/2016
    //
    ///////////////////////////////////////////////////////////
    MapHeader *AMEMapView::layoutHeader()
    {
        return &m_Header;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/18/2016
    //
    ///////////////////////////////////////////////////////////
    QVector<qboy::GLColor> *AMEMapView::mainPalettes()
    {
        return &m_Palettes[0];
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/18/2016
    //
    ///////////////////////////////////////////////////////////
    QList<UInt8 *> AMEMapView::mainPixels()
    {
        QList<UInt8 *> pixels;
        pixels.push_back(m_PrimaryBackground);
        pixels.push_back(m_PrimaryForeground);
        pixels.push_back(m_SecondaryBackground);
        pixels.push_back(m_SecondaryForeground);


        return pixels;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/18/2016
    //
    ///////////////////////////////////////////////////////////
    QSize AMEMapView::primarySetSize()
    {
        return m_PrimarySetSize;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/18/2016
    //
    ///////////////////////////////////////////////////////////
    QSize AMEMapView::secondarySetSize()
    {
        return m_SecondarySetSize;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/18/2016
    //
    ///////////////////////////////////////////////////////////
    Int32 AMEMapView::primaryBlockCount()
    {
        return m_PrimaryBlockCount;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   6/18/2016
    //
    ///////////////////////////////////////////////////////////
    Int32 AMEMapView::secondaryBlockCount()
    {
        return m_SecondaryBlockCount;
    }

    ///////////////////////////////////////////////////////////
    // Function type:  Getter
    // Contributors:   Pokedude
    // Last edit by:   Pokedude
    // Date of edit:   7/5/2016
    //
    ///////////////////////////////////////////////////////////
    MapBorder &AMEMapView::border()
    {
        return m_Header.border();
    }
}
