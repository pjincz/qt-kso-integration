/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtOpenGL module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtOpenGL/qgl.h>
#include <QtOpenGL/private/qgl_p.h>
#include <QtOpenGL/private/qglextensions_p.h>
#include "qglbuffer.h"

QT_BEGIN_NAMESPACE

/*!
    \class QGLBuffer
    \brief The QGLBuffer class provides functions for creating and managing GL buffer objects.
    \since 4.7
    \ingroup painting-3D

    Buffer objects are created in the GL server so that the
    client application can avoid uploading vertices, indices,
    texture image data, etc every time they are needed.
*/

/*!
    \enum QGLBuffer::Type
    This enum defines the type of GL buffer object to create with QGLBuffer.

    \value VertexBuffer Vertex buffer object for use when specifying
           vertex arrays.
    \value IndexBuffer Index buffer object for use with \c{glDrawElements()}.
    \value PixelPackBuffer Pixel pack buffer object for reading pixel
           data from the GL server (for example, with \c{glReadPixels()}).
           Not supported under OpenGL/ES.
    \value PixelUnpackBuffer Pixel unpack buffer object for writing pixel
           data to the GL server (for example, with \c{glTexImage2D()}).
           Not supported under OpenGL/ES.
*/

/*!
    \enum QGLBuffer::UsagePattern
    This enum defines the usage pattern of a QGLBuffer object.

    \value StreamDraw The data will be set once and used a few times
           for drawing operations.  Under OpenGL/ES 1.1 this is identical
           to StaticDraw.
    \value StreamRead The data will be set once and used a few times
           for reading data back from the GL server.  Not supported
           under OpenGL/ES.
    \value StreamCopy The data will be set once and used a few times
           for reading data back from the GL server for use in further
           drawing operations.  Not supported under OpenGL/ES.
    \value StaticDraw The data will be set once and used many times
           for drawing operations.
    \value StaticRead The data will be set once and used many times
           for reading data back from the GL server.  Not supported
           under OpenGL/ES.
    \value StaticCopy The data will be set once and used many times
           for reading data back from the GL server for use in further
           drawing operations.  Not supported under OpenGL/ES.
    \value DynamicDraw The data will be modified repeatedly and used
           many times for drawing operations.
    \value DynamicRead The data will be modified repeatedly and used
           many times for reading data back from the GL server.
           Not supported under OpenGL/ES.
    \value DynamicCopy The data will be modified repeatedly and used
           many times for reading data back from the GL server for
           use in further drawing operations.  Not supported under OpenGL/ES.
*/

/*!
    \enum QGLBuffer::Access
    This enum defines the access mode for QGLBuffer::map().

    \value ReadOnly The buffer will be mapped for reading only.
    \value WriteOnly The buffer will be mapped for writing only.
    \value ReadWrite The buffer will be mapped for reading and writing.
*/

class QGLBufferPrivate
{
public:
    QGLBufferPrivate(QGLBuffer::Type t)
        : type(t),
          guard(0),
          usagePattern(QGLBuffer::StaticDraw),
          actualUsagePattern(QGLBuffer::StaticDraw)
    {
    }

    QGLBuffer::Type type;
    QGLSharedResourceGuard guard;
    QGLBuffer::UsagePattern usagePattern;
    QGLBuffer::UsagePattern actualUsagePattern;
};

/*!
    Constructs a new buffer object of \a type.

    Note: this constructor just creates the QGLBuffer instance.  The actual
    buffer object in the GL server is not created until create() is called.

    \sa create()
*/
QGLBuffer::QGLBuffer(QGLBuffer::Type type)
    : d_ptr(new QGLBufferPrivate(type))
{
}

#define ctx d->guard.context()

/*!
    Destroys this buffer object, including the storage being
    used in the GL server.
*/
QGLBuffer::~QGLBuffer()
{
    Q_D(QGLBuffer);
    GLuint bufferId = d->guard.id();
    if (bufferId) {
        // Switch to the original creating context to destroy it.
        QGLShareContextScope scope(d->guard.context());
        glDeleteBuffers(1, &bufferId);
    }
}

/*!
    Returns the type of buffer represented by this object.
*/
QGLBuffer::Type QGLBuffer::type() const
{
    Q_D(const QGLBuffer);
    return d->type;
}

/*!
    Returns the usage pattern for this buffer object.
    The default value is StaticDraw.

    \sa setUsagePattern()
*/
QGLBuffer::UsagePattern QGLBuffer::usagePattern() const
{
    Q_D(const QGLBuffer);
    return d->usagePattern;
}

/*!
    Sets the usage pattern for this buffer object to \a value.
    This function must be called before allocate() or write().

    \sa usagePattern(), allocate(), write()
*/
void QGLBuffer::setUsagePattern(QGLBuffer::UsagePattern value)
{
    Q_D(QGLBuffer);
#if defined(QT_OPENGL_ES_1)
    // OpenGL/ES 1.1 does not support GL_STREAM_DRAW, so use GL_STATIC_DRAW.
    // OpenGL/ES 2.0 does support GL_STREAM_DRAW.
    d->usagePattern = value;
    if (value == StreamDraw)
        d->actualUsagePattern = StaticDraw;
    else
        d->actualUsagePattern = value;
#else
    d->usagePattern = d->actualUsagePattern = value;
#endif
}

#undef ctx

/*!
    Creates the buffer object in the GL server.  Returns true if
    the object was created; false otherwise.

    This function must be called with a current QGLContext.
    The buffer will be bound to and can only be used in
    that context (or any other context that is shared with it).

    This function will return false if the GL implementation
    does not support buffers, or there is no current QGLContext.

    \sa isCreated(), allocate(), write()
*/
bool QGLBuffer::create()
{
    Q_D(QGLBuffer);
    if (d->guard.id())
        return true;
    const QGLContext *ctx = QGLContext::currentContext();
    if (ctx) {
        if (!qt_resolve_buffer_extensions(const_cast<QGLContext *>(ctx)))
            return false;
        GLuint bufferId = 0;
        glGenBuffers(1, &bufferId);
        if (bufferId) {
            d->guard.setContext(ctx);
            d->guard.setId(bufferId);
            return true;
        }
    }
    return false;
}

#define ctx d->guard.context()

/*!
    Returns true if this buffer has been created; false otherwise.

    \sa create()
*/
bool QGLBuffer::isCreated() const
{
    Q_D(const QGLBuffer);
    return d->guard.id() != 0;
}

/*!
    Reads the \a count bytes in this buffer starting at \a offset
    into \a data.  Returns true on success; false if reading from
    the buffer is not supported.  Buffer reading is not supported
    under OpenGL/ES.

    It is assumed that this buffer has been bound to the current context.

    \sa write(), bind()
*/
bool QGLBuffer::read(int offset, void *data, int count)
{
#if !defined(QT_OPENGL_ES)
    Q_D(QGLBuffer);
    if (!glGetBufferSubData || !d->guard.id())
        return false;
    while (glGetError() != GL_NO_ERROR) ; // Clear error state.
    glGetBufferSubData(d->type, offset, count, data);
    return glGetError() == GL_NO_ERROR;
#else
    Q_UNUSED(offset);
    Q_UNUSED(data);
    Q_UNUSED(count);
    return false;
#endif
}

/*!
    Replaces the \a count bytes of this buffer starting at \a offset
    with the contents of \a data.  Any other bytes in the buffer
    will be left unmodified.

    It is assumed that create() has been called on this buffer and that
    it has been bound to the current context.

    \sa create(), read(), allocate()
*/
void QGLBuffer::write(int offset, const void *data, int count)
{
    Q_D(QGLBuffer);
    if (d->guard.id())
        glBufferSubData(d->type, offset, count, data);
}

/*!
    Allocates \a count bytes of space to the buffer, initialized to
    the contents of \a data.  Any previous contents will be removed.

    It is assumed that create() has been called on this buffer and that
    it has been bound to the current context.

    \sa create(), read(), write()
*/
void QGLBuffer::allocate(const void *data, int count)
{
    Q_D(QGLBuffer);
    if (d->guard.id())
        glBufferData(d->type, count, data, d->actualUsagePattern);
}

/*!
    \fn void QGLBuffer::allocate(int count)
    \overload

    Allocates \a count bytes of space to the buffer.  Any previous
    contents will be removed.

    It is assumed that create() has been called on this buffer and that
    it has been bound to the current context.

    \sa create(), write()
*/

/*!
    Binds the buffer associated with this object to the current
    GL context.  Returns false if binding was not possible, usually because
    type() is not supported on this GL implementation.

    The buffer must be bound to the same QGLContext current when create()
    was called, or to another QGLContext that is sharing with it.
    Otherwise, false will be returned from this function.

    \sa release(), create()
*/
bool QGLBuffer::bind() const
{
    Q_D(const QGLBuffer);
    GLuint bufferId = d->guard.id();
    if (bufferId) {
        if (!QGLContext::areSharing(QGLContext::currentContext(),
                                    d->guard.context())) {
#ifndef QT_NO_DEBUG
            qWarning("QGLBuffer::bind: buffer is not valid in the current context");
#endif
            return false;
        }
        glBindBuffer(d->type, bufferId);
        return true;
    } else {
        return false;
    }
}

/*!
    Releases the buffer associated with this object from the
    current GL context.

    This function must be called with the same QGLContext current
    as when bind() was called on the buffer.

    \sa bind()
*/
void QGLBuffer::release() const
{
    Q_D(const QGLBuffer);
    if (d->guard.id())
        glBindBuffer(d->type, 0);
}

/*!
    Returns the GL identifier associated with this buffer; zero if
    the buffer has not been created.

    \sa isCreated()
*/
uint QGLBuffer::bufferId() const
{
    Q_D(const QGLBuffer);
    return d->guard.id();
}

#ifndef GL_BUFFER_SIZE
#define GL_BUFFER_SIZE 0x8764
#endif

/*!
    Returns the size of the data in this buffer, for reading operations.
    Returns -1 if fetching the buffer size is not supported, or the
    buffer has not been created.

    It is assumed that this buffer has been bound to the current context.

    \sa isCreated(), bind()
*/
int QGLBuffer::size() const
{
    Q_D(const QGLBuffer);
    if (!d->guard.id())
        return -1;
    GLint value = -1;
    glGetBufferParameteriv(d->type, GL_BUFFER_SIZE, &value);
    return value;
}

/*!
    Maps the contents of this buffer into the application's memory
    space and returns a pointer to it.  Returns null if memory
    mapping is not possible.  The \a access parameter indicates the
    type of access to be performed.

    It is assumed that create() has been called on this buffer and that
    it has been bound to the current context.

    This function is only supported under OpenGL/ES if the
    \c{GL_OES_mapbuffer} extension is present.

    \sa unmap(), create(), bind()
*/
void *QGLBuffer::map(QGLBuffer::Access access)
{
    Q_D(QGLBuffer);
    if (!d->guard.id())
        return 0;
    if (!glMapBufferARB)
        return 0;
    return glMapBufferARB(d->type, access);
}

/*!
    Unmaps the buffer after it was mapped into the application's
    memory space with a previous call to map().  Returns true if
    the unmap succeeded; false otherwise.

    It is assumed that this buffer has been bound to the current context,
    and that it was previously mapped with map().

    This function is only supported under OpenGL/ES if the
    \c{GL_OES_mapbuffer} extension is present.

    \sa map()
*/
bool QGLBuffer::unmap()
{
    Q_D(QGLBuffer);
    if (!d->guard.id())
        return false;
    if (!glUnmapBufferARB)
        return false;
    return glUnmapBufferARB(d->type) == GL_TRUE;
}

QT_END_NAMESPACE