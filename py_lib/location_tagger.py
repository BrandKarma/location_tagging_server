from analyzer_wrapper import AnalyzerClient

class AnalyzerWrapper:
    def __init__(self, hostname, port):
        self.client = AnalyzerClient(hostname, port)
        self.text_parser = TextParser()

    def get_overrides_by_concept(self, overrides, sentences):
        sentence_id = 1
        adjustment = 0
        overrides_by_concept = {}
        inside = False
        for override in overrides:
            if not overrides_by_concept.has_key(override["concept_id"]):
                overrides_by_concept[override["concept_id"]] = []
            os = override["start_position"]
            oe = override["end_position"]
            window_entrance = os
            while True:
                if sentence_id >= len(sentences):
                    break
                else:
                    # This logic tries to figure out which sentence(s) this particular override applies to
                    # Pretty lame hack-y error prone thing, but we're limited by the current interface
                    ss = sentences[sentence_id].position.start_position - self.text_parser.title_offset
                    se = sentences[sentence_id].position.end_position - self.text_parser.title_offset

                    if ss < (os + adjustment):
                        sentence_id += 1
                        inside = False
                    else:
                        if se > ss:
                            if (oe + adjustment) <= se:
                                overrides_by_concept[override["concept_id"]].append(sentence_id)
                                if (oe + adjustment) < se:
                                    inside = True
                                else:
                                    inside = False
                                    break
                                sentence_id += 1
                            else:
                                if inside and ss <= (os + adjustment):
                                    # The adjustment started before this current sentence started, so even though
                                    # it runs over we'll include it and then adjust the adjustment
                                    overrides_by_concept[override["concept_id"]].append(sentence_id)
                                    adjustment -= ((oe + adjustment) - se)
                                    sentence_id += 1
                                    inside = False
                                else:
                                    break

        return overrides_by_concept

    def get_scores(self,
                   subject_id,
                   title,
                   body,
                   brand_name,
                   language_id,
                   persona_id=0,
                   concepts_to_score=None,
                   supplemental_weighted=None,
                   supplemental=None):
        sentences = []
        paragraphs = []
        text = self.text_parser.parse_text(title, body, sentences, paragraphs, True)
        input = AnalyzerInput(text, sentences, paragraphs, brand_name, persona_id, language_id, self.text_parser.title_offset, supplemental_weighted=supplemental_weighted, supplemental=supplemental)
        output = self.client.analyze(input, concepts_to_score)

        output.add_relevant_sentence_info(input)

        return output


    def get_translated_scores(self, subject_id, title, body, brand_name,
            source_language_id, translated_language_id, google_api_key=None, persona_id=0,
            concepts_to_score=None, supplemental_weighted=None, supplemental=None, debug=False):
        origin_sentences = []
        origin_paragraphs = []

        text = self.text_parser.parse_text(title, body, origin_sentences, origin_paragraphs, True)
        if not LANGUAGE_ID_LOOKUPS.has_key(source_language_id):
            raise AnalyzerException(103, "Unable to translate from source language_id: %d" % source_language_id)
        origin = LANGUAGE_ID_LOOKUPS[source_language_id]

        if not LANGUAGE_ID_LOOKUPS.has_key(translated_language_id):
            raise AnalyzerException(104, "Unable to translate to destination language_id: %d" % translated_language_id)
        target = LANGUAGE_ID_LOOKUPS[translated_language_id]

        translated_text, translated_sentences, translated_paragraphs = get_translated_sentence_data(text, origin_sentences, origin, target, google_api_key)

        translated_input = AnalyzerInput(translated_text, translated_sentences, translated_paragraphs,
            brand_name, persona_id, translated_language_id, translated_sentences[0].position.end_position,
            supplemental_weighted=supplemental_weighted, supplemental=supplemental)
        output = self.client.analyze(translated_input, concepts_to_score)
        output.revert_translated_sentences(text, origin_sentences, origin_paragraphs)
        return output


    def get_scores_with_hint(self,
                             hint_id,
                             text,
                             brand_name,
                             language_id,
                             persona_id=0,
                             concepts_to_score=None,
                             supplemental_weighted=None,
                             supplemental=None):
        sentences = []
        paragraphs = []
        text = self.text_parser.parse_text("", text, sentences, paragraphs, True)
        input = AnalyzerInput(text,
                              sentences,
                              paragraphs,
                              brand_name,
                              persona_id,
                              language_id,
                              self.text_parser.title_offset,
                              supplemental_weighted=supplemental_weighted,
                              supplemental=supplemental,
                              analyzer_hint=hint_id)
        output = self.client.analyze(input, concepts_to_score)

        output.add_relevant_sentence_info(input)
        return output
