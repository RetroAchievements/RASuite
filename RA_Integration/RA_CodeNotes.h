#pragma once

#include "RA_Defs.h"

class CodeNotes
{
public:
	class CodeNoteObj
	{
	public:
		CodeNoteObj( const std::string& sAuthor, std::string sNote ) : 
		  m_sAuthor( sAuthor ), m_sNote( sNote ) {}

	public:
		const std::string& Author() const			{ return m_sAuthor; }
		const std::string& Note() const				{ return m_sNote; }

		void SetNote( const std::string& sNote )	{ m_sNote = sNote; }

	private:
		const std::string m_sAuthor;
		std::string m_sNote;
	};
	
public:
	void Clear();

	bool Save( const std::string& sFile );
	size_t Load( const std::string& sFile );

	bool ReloadFromWeb( GameID nID );
	static void OnCodeNotesResponse( Document& doc );

	void Add( const ByteAddress& nAddr, const std::string& sAuthor, const std::string& sNote );
	bool Remove( const ByteAddress& nAddr );
	
	const CodeNoteObj* FindCodeNote( const ByteAddress& nAddr ) const		{ return( m_CodeNotes.find( nAddr ) != m_CodeNotes.end() ) ? &m_CodeNotes.at( nAddr ) : nullptr; }

	std::map<ByteAddress, CodeNoteObj>::const_iterator FirstNote() const	{ return m_CodeNotes.begin(); }
	std::map<ByteAddress, CodeNoteObj>::const_iterator EndOfNotes() const	{ return m_CodeNotes.end(); }

private:
	std::map<ByteAddress, CodeNoteObj> m_CodeNotes;
};